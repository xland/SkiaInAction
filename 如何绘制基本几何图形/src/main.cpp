#include <windows.h>
#include <string>
#include "include/core/SkSurface.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkRRect.h"

int w{800}, h{600};
std::vector<SkColor> surfaceMemory;

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

void drawLine(SkCanvas *canvas)
{
    SkPaint paint;
    paint.setColor(SK_ColorRED);
    paint.setAntiAlias(true);
    paint.setStroke(true);
    paint.setStrokeWidth(16);
    paint.setStrokeCap(SkPaint::Cap::kRound_Cap);
    canvas->drawLine(80, 80, w - 80, h - 80, paint);
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

void drawPoint(SkCanvas *canvas)
{
    SkPaint paint;
    paint.setColor(SK_ColorRED);
    paint.setAntiAlias(true);
    paint.setStroke(true);
    paint.setStrokeWidth(16);
    paint.setStrokeCap(SkPaint::Cap::kRound_Cap);
    // canvas->drawPoint(w/2,h/2, paint);

    SkPoint pts[]{SkPoint::Make(60, 60),
                  SkPoint::Make(w / 2, h / 2),
                  SkPoint::Make(w - 60, h - 60)};
    canvas->drawPoints(SkCanvas::PointMode::kPoints_PointMode, 3, pts, paint);
}

void drawPath(SkCanvas *canvas)
{
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setColor(SK_ColorRED);
    paint.setStroke(true);
    paint.setStrokeWidth(16);
    paint.setStrokeJoin(SkPaint::kRound_Join);
    SkPath path;
    path.moveTo(60, 120);
    path.lineTo(180, 60);
    path.lineTo(w - 60, 120);
    path.lineTo(w - 160, h - 160);
    path.lineTo(180, h - 60);
    path.close();
    canvas->drawPath(path, paint);
}

void drawBezierPath(SkCanvas *canvas)
{
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setColor(SK_ColorRED);
    paint.setStroke(true);
    paint.setStrokeCap(SkPaint::Cap::kRound_Cap);
    paint.setStrokeWidth(6);
    SkPath path;
    path.moveTo(60, 60);
    path.cubicTo(280, 60, w - 280, h - 60, w - 60, h - 60);
    canvas->drawPath(path, paint);
}

void setPixel()
{
    surfaceMemory.resize(w * h, 0xff000000);
    SkImageInfo info = SkImageInfo::MakeN32Premul(w, h);
    auto canvas = SkCanvas::MakeRasterDirect(info, surfaceMemory.get(), 4 * w);
    drawCircle(canvas.get());
    // drawEllipse(canvas.get());
    // drawRRect(canvas.get());
    // drawLine(canvas.get());
    // drawArc(canvas.get());
    // drawPoint(canvas.get());
    // drawPath(canvas.get());
    // drawBezierPath(canvas.get());
}

void paint(const HWND hWnd)
{
    PAINTSTRUCT ps;
    auto dc = BeginPaint(hWnd, &ps);
    BITMAPINFO info = {sizeof(BITMAPINFOHEADER), w, 0 - h, 1, 32, BI_RGB, h * 4 * w, 0, 0, 0, 0};
    StretchDIBits(dc, 0, 0, w, h, 0, 0, w, h, surfaceMemory.get(), &info, DIB_RGB_COLORS, SRCCOPY);
    ReleaseDC(hWnd, dc);
    EndPaint(hWnd, &ps);
    std::vector<SkColor> vec;
    surfaceMemory.swap(vec);
}

LRESULT CALLBACK wndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_SIZE:
    {
        w = LOWORD(lParam);
        h = HIWORD(lParam);
        if (wParam != SIZE_MINIMIZED)
        {
            setPixel();
        }
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