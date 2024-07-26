#include <windows.h>
#include <string>
#include "include/core/SkSurface.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkRRect.h"
#include "include/core/SkPath.h"
#include "include/pathops/SkPathOps.h"
#include "include/core/SkPathEffect.h"
#include "include/effects/SkDashPathEffect.h"
#include "include/effects/SkDiscretePathEffect.h"
#include "include/effects/SkGradientShader.h"

int w{800}, h{600};

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

void drawMultiPath(SkCanvas* canvas)
{
    SkPath path0;
    path0.addRect(SkRect::MakeXYWH(100, 100, 100, 100));
    SkPath path1;
    path1.addRect(SkRect::MakeXYWH(150, 150, 100, 100));
    SkPath path;
    Op(path0, path1, SkPathOp::kXOR_SkPathOp, &path);
    SkPaint paint;
    paint.setColor(0xFF00FFFF);
    canvas->drawPath(path, paint);
}

void drawDashPath(SkCanvas* canvas)
{
    SkPaint paint;
    paint.setColor(0xFF00FFFF);
    paint.setStroke(true);
    paint.setStrokeWidth(8);
    paint.setStrokeCap(SkPaint::Cap::kRound_Cap);
    paint.setStrokeJoin(SkPaint::kRound_Join);
    paint.setAntiAlias(true);
    SkScalar kIntervals[] = { 12, 16, 28, 18 };
    sk_sp<SkPathEffect> effect = SkDashPathEffect::Make(kIntervals, 4, 25);
    paint.setPathEffect(effect);
    SkPath path;
    path.moveTo(60, 120);
    path.lineTo(180, 60);
    path.lineTo(w - 60, 120);
    path.lineTo(w - 160, h - 160);
    path.lineTo(180, h - 60);
    path.close();
    canvas->drawPath(path, paint);
}

void drawPathMask(SkCanvas* canvas)
{
    SkPaint paint;
    SkPoint pts[2]{ SkPoint::Make(0, 0), SkPoint::Make(w, h) };
    SkColor colors[6]{ 0xFF00FFFF, 0xFFFF00FF, 0xFFFFFF00, 0xFF0000FF, 0xFF00FF00, 0xFFFF0000 };
    sk_sp<SkShader> shader = SkGradientShader::MakeLinear(pts, colors, nullptr, 6, SkTileMode::kClamp);
    paint.setShader(shader);
    canvas->drawPaint(paint);
    paint.setShader(nullptr);
    paint.setColor(0xAA000000);
    SkPath path;
    SkRect rect = SkRect::MakeLTRB(100, 100, w-100, h-100);
    path.addRect(rect);
    path.setFillType(SkPathFillType::kInverseEvenOdd);

    //bool flag = path.contains(w / 2, h / 2);  false

    canvas->drawPath(path, paint);
}

void paint(const HWND hWnd)
{
    if (w <= 0 || h <= 0)
        return;
    SkColor *surfaceMemory = new SkColor[w * h]{0xff000000};
    SkImageInfo info = SkImageInfo::MakeN32Premul(w, h);
    auto canvas = SkCanvas::MakeRasterDirect(info, surfaceMemory, 4 * w);
    // drawPath(canvas.get());
    // drawBezierPath(canvas.get());
    // drawMultiPath(canvas.get());
    //drawDashPath(canvas.get());
    drawPathMask(canvas.get());
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