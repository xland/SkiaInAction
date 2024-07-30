#include <windows.h>
#include <string>
#include "include/core/SkSurface.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkRRect.h"
#include "include/core/SkPath.h"
#include "include/core/SkPoint.h"

int w{800}, h{600};

void drawRect(SkCanvas* canvas) {
    SkPaint paint;
    paint.setColor(SK_ColorRED);
    paint.setStroke(true);
    paint.setStrokeWidth(16);
    SkRect rect = SkRect::MakeLTRB(w / 2 - 100, h / 2 - 100, w / 2 + 100, h / 2 + 100);
    bool isPointInRect = rect.contains(w / 2, h / 2);
    canvas->drawRect(rect, paint);
}

void drawPoint(SkCanvas* canvas)
{
    SkPaint paint;
    paint.setColor(SK_ColorRED);
    paint.setStrokeWidth(16);
    canvas->drawPoint(w/2,h/2, paint);
}

void drawPoint2(SkCanvas* canvas)
{
    SkPaint paint;
    paint.setColor(SK_ColorRED);
    paint.setStrokeWidth(16);
    SkPoint pts[]{ SkPoint::Make(60, 60),
                  SkPoint::Make(w / 2, h / 2),
                  SkPoint::Make(w - 60, h - 60) };
    canvas->drawPoints(SkCanvas::PointMode::kPoints_PointMode, 3, pts, paint);
}

void drawLine(SkCanvas* canvas)
{
    SkPaint paint;
    paint.setColor(SK_ColorRED);
    paint.setAntiAlias(true);
    paint.setStroke(true);
    paint.setStrokeWidth(16);
    paint.setStrokeCap(SkPaint::Cap::kRound_Cap);
    canvas->drawLine(80, 80, w - 80, h - 80, paint);
}

void drawCircle(SkCanvas *canvas)
{
    SkPaint paint;
    paint.setColor(SK_ColorRED);
    paint.setAntiAlias(true);
    auto x = w / 2;
    auto y = h / 2;
    auto r = std::min(x - 10, y - 10);
    canvas->drawCircle(x, y, r, paint);
}

void drawEllipse(SkCanvas *canvas)
{
    SkPaint paint;
    paint.setColor(SK_ColorRED);
    paint.setAntiAlias(true);
    SkRect rect;
    rect.setLTRB(10, 10, w - 10, h - 10);
    canvas->drawOval(rect, paint);
}

void drawRRect(SkCanvas *canvas)
{
    SkPaint paint;
    paint.setColor(SK_ColorRED);
    paint.setAntiAlias(true);
    SkRect rect;
    rect.setLTRB(60, 60, w - 60, h - 60);
    SkVector radii[4]{
        {16, 16}, // 矩形左上角圆角尺寸；
        {16, 16}, // 矩形右上角圆角尺寸；
        {16, 16}, // 矩形右下角圆角尺寸；
        {16, 16}  // 矩形左下角圆角尺寸；
    };
    SkRRect rr;
    rr.setRectRadii(rect, radii);
    canvas->drawRRect(rr, paint);
}

void drawArc(SkCanvas *canvas)
{
    SkPaint paint;
    paint.setColor(SK_ColorRED);
    paint.setAntiAlias(true);
    paint.setStroke(true);
    paint.setStrokeWidth(16);
    paint.setStrokeCap(SkPaint::Cap::kRound_Cap);
    SkRect rect;
    rect.setLTRB(60, 60, w - 60, h - 60);
    canvas->drawArc(rect, 0, -90, false, paint);
}

void paint(const HWND hWnd)
{
    if (w <= 0 || h <= 0)
        return;
    SkColor *surfaceMemory = new SkColor[w * h]{0xff000000};
    SkImageInfo info = SkImageInfo::MakeN32Premul(w, h);
    auto canvas = SkCanvas::MakeRasterDirect(info, surfaceMemory, 4 * w);
    drawRect(canvas.get());
     //drawPoint(canvas.get());
    //drawPoint2(canvas.get());
    // drawLine(canvas.get());
    // drawCircle(canvas.get());
    // drawEllipse(canvas.get());
    // drawRRect(canvas.get());
    // drawArc(canvas.get());

    PAINTSTRUCT ps;
    auto dc = BeginPaint(hWnd, &ps);
    BITMAPINFO bmpInfo = {sizeof(BITMAPINFOHEADER), w, 0 - h, 1, 32, BI_RGB, h * 4 * w, 0, 0, 0, 0};
    StretchDIBits(dc, 0, 0, w, h, 0, 0, w, h, surfaceMemory, &bmpInfo, DIB_RGB_COLORS, SRCCOPY);
    ReleaseDC(hWnd, dc);
    EndPaint(hWnd, &ps);
    delete[] surfaceMemory;
}

LRESULT CALLBACK wndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_SIZE:
    {
        w = LOWORD(lParam);
        h = HIWORD(lParam);
        break;
    }
    case WM_PAINT:
    {
        paint(hWnd);
        break;
    }
    case WM_CLOSE:
    {
        PostQuitMessage(0);
        break;
    }
    default:
    {
        break;
    }
    }
    return DefWindowProc(hWnd, message, wParam, lParam);
}

void initWindow()
{
    std::wstring clsName{L"DrawInWindow"};
    auto hinstance = GetModuleHandle(NULL);
    WNDCLASSEX wcx{};
    wcx.cbSize = sizeof(WNDCLASSEX);
    wcx.style = CS_HREDRAW | CS_VREDRAW;
    wcx.lpfnWndProc = wndProc;
    wcx.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcx.lpszClassName = clsName.c_str();
    if (!RegisterClassEx(&wcx))
    {
        return;
    }
    auto hwnd = CreateWindow(clsName.c_str(), clsName.c_str(), WS_OVERLAPPEDWINDOW,
                             CW_USEDEFAULT, CW_USEDEFAULT, w, h,
                             nullptr, nullptr, hinstance, nullptr);
    ShowWindow(hwnd, SW_SHOW);
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPTSTR lpCmdLine, _In_ int nCmdShow)
{
    initWindow();
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}