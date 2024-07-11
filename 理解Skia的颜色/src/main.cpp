#include <windows.h>
#include <string>
#include "include/core/SkSurface.h"
#include "include/core/SkCanvas.h"
#include "src/base/SkAutoMalloc.h"
#include "include/core/SkRRect.h"
#include "include/effects/SkGradientShader.h"
#include "include/effects/SkPerlinNoiseShader.h"

#include "include/core/SkBitmap.h"

int w{500}, h{500};
SkAutoMalloc surfaceMemory;

void drawLinearGradientColor(SkCanvas *canvas)
{
    SkPaint paint;
    paint.setAntiAlias(true);
    SkPoint pts[2]{SkPoint::Make(0, 0), SkPoint::Make(w, h)};
    SkColor colors[6]{ 0xFF00FFFF, 0xFFFF00FF, 0xFFFFFF00,0xFF0000FF,0xFF00FF00,0xFFFF0000 };
    auto shader = SkGradientShader::MakeLinear(pts, colors, nullptr, 6, SkTileMode::kClamp);
    paint.setShader(shader);
    canvas->drawPaint(paint);
    //auto x = w / 2;
    //auto y = h / 2;
    //auto r = std::min(x - 60, y - 60);
    //canvas->drawCircle(x, y, r, paint);
}

void drawRadialGradientColor(SkCanvas *canvas)
{
    SkPaint paint;
    paint.setAntiAlias(true);
    SkColor colors[6]{ 0xFF00FFFF, 0xFFFF00FF, 0xFFFFFF00,0xFF0000FF,0xFF00FF00,0xFFFF0000 };
    auto x = w / 2;
    auto y = h / 2;
    auto r = std::min(x - 10, y - 10);
    auto shader = SkGradientShader::MakeRadial(SkPoint::Make(x, y), r, colors, nullptr, 6, SkTileMode::kClamp);
    paint.setShader(shader);
    canvas->drawPaint(paint);
}

void drawConicalGradientColor(SkCanvas* canvas)
{
    SkPaint paint;
    auto x = w / 2;
    auto y = h / 2;
    SkColor colors[6]{ 0xFF00FFFF, 0xFFFF00FF, 0xFFFFFF00,0xFF0000FF,0xFF00FF00,0xFFFF0000 };
    auto shader = SkGradientShader::MakeTwoPointConical(SkPoint::Make(x, y), y, SkPoint::Make(x, 60.0f), 20.0f,
        colors, nullptr, 6, SkTileMode::kClamp);
    paint.setShader(shader);
    canvas->drawPaint(paint);
}

void drawSweepGradientColor(SkCanvas* canvas)
{
    SkPaint paint;
    auto x = w / 2;
    auto y = h / 2;
    SkColor colors[7]{ 0xFF00FFFF, 0xFFFF00FF, 0xFFFFFF00,0xFF0000FF,0xFF00FF00,0xFFFF0000,0xFF00FFFF };
    auto shader = SkGradientShader::MakeSweep(x, y, colors, nullptr, 7, 0, nullptr);
    paint.setShader(shader);
    canvas->drawPaint(paint);
}

void drawNoiseColor(SkCanvas* canvas) {
    canvas->clear(SK_ColorWHITE);
    SkPaint paint;
    auto shader = SkShaders::MakeFractalNoise(0.1f, 0.1f, 6, 0.0f, nullptr);
    //auto shader = SkShaders::MakeTurbulence(0.1f, 0.1f, 6, 0.0f, nullptr);
    paint.setShader(shader);
    canvas->drawPaint(paint);
}

void averageColor(SkCanvas* canvas) {
    SkColor colorArr[6]{ 0xFF123456,0xFF654321,0xFF789ABC,0xFFABC789,0xFFDEF123,0xFF123DEF };
    SkColor4f averageColor{ SkColors::kTransparent };
    for (size_t i = 0; i < 6; i++)
    {
        auto tempColor = SkColor4f::FromColor(colorArr[i]);
        averageColor.fA += tempColor.fA;
        averageColor.fR += tempColor.fR;
        averageColor.fG += tempColor.fG;
        averageColor.fB += tempColor.fB;
    }
    averageColor.fR /= 6;
    averageColor.fG /= 6;
    averageColor.fB /= 6;
    averageColor.fA /= 6;
    auto color = averageColor.toSkColor();
    canvas->clear(averageColor);
}

void colorOverlay(SkCanvas* canvas) {
    SkBitmap bitmap;
    bitmap.allocN32Pixels(1, 1);
    SkCanvas tempCanvas(bitmap);
    tempCanvas.drawColor(0x88DD3456);
    tempCanvas.drawColor(0x88654321);
    void* pixels = bitmap.getPixels();
    auto result = *(SkColor*)pixels;
    canvas->clear(result);
}

void setPixel()
{
    surfaceMemory.reset(h * 4 * w);
    SkImageInfo info = SkImageInfo::MakeN32Premul(w, h);
    auto canvas = SkCanvas::MakeRasterDirect(info, surfaceMemory.get(), 4 * w);
    canvas->clear(SK_ColorBLACK);
    drawLinearGradientColor(canvas.get());
    //drawRadialGradientColor(canvas.get());
    //drawConicalGradientColor(canvas.get());
    //drawSweepGradientColor(canvas.get());
    //drawNoiseColor(canvas.get());
    //averageColor(canvas.get());
    colorOverlay(canvas.get());
}

void paint(const HWND hWnd)
{
    PAINTSTRUCT ps;
    auto dc = BeginPaint(hWnd, &ps);
    BITMAPINFO info = {sizeof(BITMAPINFOHEADER), w, 0 - h, 1, 32, BI_RGB, h * 4 * w, 0, 0, 0, 0};
    StretchDIBits(dc, 0, 0, w, h, 0, 0, w, h, surfaceMemory.get(), &info, DIB_RGB_COLORS, SRCCOPY);
    ReleaseDC(hWnd, dc);
    EndPaint(hWnd, &ps);
    surfaceMemory.reset(0);
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