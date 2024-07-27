#include <windows.h>
#include <string>
#include "include/core/SkSurface.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkRRect.h"
#include "include/effects/SkGradientShader.h"
#include "include/effects/SkPerlinNoiseShader.h"
#include "include/effects/SkColorMatrix.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkPath.h"

#include <vector>
#include <format>
#include <sstream>

int w{500}, h{500};

void drawLinearGradientColor(SkCanvas *canvas)
{
    SkPaint paint;
    paint.setAntiAlias(true);
    SkPoint pts[2]{SkPoint::Make(0, 0), SkPoint::Make(w, h)};
    SkColor colors[6]{0xFF00FFFF, 0xFFFF00FF, 0xFFFFFF00, 0xFF0000FF, 0xFF00FF00, 0xFFFF0000};
    auto shader = SkGradientShader::MakeLinear(pts, colors, nullptr, 6, SkTileMode::kClamp);
    paint.setShader(shader);
    canvas->drawPaint(paint);
    // auto x = w / 2;
    // auto y = h / 2;
    // auto r = std::min(x - 60, y - 60);
    // canvas->drawCircle(x, y, r, paint);
}

void drawRadialGradientColor(SkCanvas *canvas)
{
    SkPaint paint;
    paint.setAntiAlias(true);
    SkColor colors[6]{0xFF00FFFF, 0xFFFF00FF, 0xFFFFFF00, 0xFF0000FF, 0xFF00FF00, 0xFFFF0000};
    auto x = w / 2;
    auto y = h / 2;
    auto r = std::min(x - 10, y - 10);
    auto shader = SkGradientShader::MakeRadial(SkPoint::Make(x, y), r, colors, nullptr, 6, SkTileMode::kClamp);
    paint.setShader(shader);
    canvas->drawPaint(paint);
}

void drawConicalGradientColor(SkCanvas *canvas)
{
    SkPaint paint;
    auto x = w / 2;
    auto y = h / 2;
    SkColor colors[6]{0xFF00FFFF, 0xFFFF00FF, 0xFFFFFF00, 0xFF0000FF, 0xFF00FF00, 0xFFFF0000};
    auto shader = SkGradientShader::MakeTwoPointConical(SkPoint::Make(x, y), y, SkPoint::Make(x, 60.0f), 20.0f,
                                                        colors, nullptr, 6, SkTileMode::kClamp);
    paint.setShader(shader);
    canvas->drawPaint(paint);
}

void drawSweepGradientColor(SkCanvas *canvas)
{
    SkPaint paint;
    auto x = w / 2;
    auto y = h / 2;
    SkColor colors[7]{0xFF00FFFF, 0xFFFF00FF, 0xFFFFFF00, 0xFF0000FF, 0xFF00FF00, 0xFFFF0000, 0xFF00FFFF};
    auto shader = SkGradientShader::MakeSweep(x, y, colors, nullptr, 7, 0, nullptr);
    paint.setShader(shader);
    canvas->drawPaint(paint);
}

void drawNoiseColor(SkCanvas *canvas)
{
    canvas->clear(SK_ColorWHITE);
    SkPaint paint;
    auto shader = SkShaders::MakeFractalNoise(0.1f, 0.1f, 6, 0.0f, nullptr);
    // auto shader = SkShaders::MakeTurbulence(0.1f, 0.1f, 6, 0.0f, nullptr);
    paint.setShader(shader);
    canvas->drawPaint(paint);
}

void colorFilter(SkCanvas *canvas)
{
    SkPaint paint;
    auto x = w / 2;
    auto y = h / 2;
    // 绘制锥型渐变的代码
    SkColor colors[6]{0xFF00FFFF, 0xFFFFFF66, 0xFFFF00FF, 0xFF66FFFF, 0xFFFFFF00, 0xFFFF66FF};
    auto shader = SkGradientShader::MakeTwoPointConical(SkPoint::Make(x, y), y, SkPoint::Make(x, 60.0f), 20.0f,
                                                        colors, nullptr, 6, SkTileMode::kClamp);
    paint.setShader(shader);
    // 颜色矩阵过滤
    SkColorMatrix colorMatrix;
    colorMatrix.setSaturation(0);
    auto filter = SkColorFilters::Matrix(colorMatrix);
    paint.setColorFilter(filter);
    canvas->drawPaint(paint);
}

void drawCylinder(SkCanvas* canvas) {

    int bodyW{ 160 }, top{60};
    int x1{ w / 2 - bodyW / 2 }, x2{ w / 2 + bodyW / 2 };

    SkPath path;
    SkRect rect;
    rect.setXYWH(x1, h - 80, bodyW, 40);
    path.arcTo(rect, 0, 180,true);
    path.lineTo(x1, top+20);
    path.lineTo(x2, top+20);
    path.close();

    SkPaint paint;
    paint.setAntiAlias(true);

    SkPoint pts[2]{ SkPoint::Make(x1, top), SkPoint::Make(x2, top) };
    SkColor colors[2]{ 0xFF287191, 0xFF85B5CB };
    sk_sp<SkShader> shader = SkGradientShader::MakeLinear(pts, colors, nullptr, 2, SkTileMode::kClamp);
    paint.setShader(shader);
    canvas->drawPath(path, paint);

    rect.setXYWH(x1, top, bodyW, 40);
    colors[0] = 0xffBDE4F8;
    colors[1] = 0xffA7CFE6;
    pts[0] = SkPoint::Make(x1, top + 40);
    pts[1] = SkPoint::Make(x2, top);
    shader = SkGradientShader::MakeLinear(pts, colors, nullptr, 2, SkTileMode::kClamp);
    paint.setShader(shader);
    canvas->drawOval(rect, paint);    
}

void paint(const HWND hWnd)
{
    if (w <= 0 || h <= 0)
        return;
    SkColor *surfaceMemory = new SkColor[w * h]{0xff000000};
    SkImageInfo info = SkImageInfo::MakeN32Premul(w, h);
    auto canvas = SkCanvas::MakeRasterDirect(info, surfaceMemory, 4 * w);
     //drawLinearGradientColor(canvas.get());
    // drawRadialGradientColor(canvas.get());
    // drawConicalGradientColor(canvas.get());
    // drawSweepGradientColor(canvas.get());
    drawCylinder(canvas.get());
    // drawNoiseColor(canvas.get());
    // colorFilter(canvas.get());


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