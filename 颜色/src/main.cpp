#include <windows.h>
#include <string>
#include "include/core/SkSurface.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkRRect.h"
#include "include/effects/SkGradientShader.h"
#include "include/effects/SkPerlinNoiseShader.h"
#include "include/effects/SkColorMatrix.h"
#include "include/core/SkColorFilter.h"
#include <vector>
#include "include/core/SkBitmap.h"
#include <format>
#include <sstream>

int w{500}, h{500};

void averageColor(SkCanvas *canvas)
{
    SkColor colorArr[6]{0xFF123456, 0xFF654321, 0xFF789ABC, 0xFFABC789, 0xFFDEF123, 0xFF123DEF};
    SkColor4f averageColor{SkColors::kTransparent};
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

void colorOverlay(SkCanvas *canvas)
{
    SkBitmap bitmap;
    bitmap.allocN32Pixels(1, 1);
    SkCanvas tempCanvas(bitmap);
    tempCanvas.drawColor(0x88DD3456);
    tempCanvas.drawColor(0x88654321);
    auto result = bitmap.getColor(0, 0);
    canvas->clear(result);
}

void formatColor()
{
    auto color = SkColor4f::FromColor(0x99887766);
    int R{(int)(color.fR * 255)},
        G{(int)(color.fG * 255)},
        B{(int)(color.fB * 255)},
        A{(int)(color.fA * 255)};
    auto colorStr = std::format("RGBA: {},{},{},{}", R, G, B, A); // RGBA: 136,119,102,153

    std::stringstream ss;
    ss << std::hex << ((R << 24) | (G << 16) | (B << 8) | A);
    ;
    std::string hex = ss.str();
    std::transform(hex.begin(), hex.end(), hex.begin(), toupper);
    colorStr = std::format("HEX: #{}", hex); // HEX: #88776699
}

void drawBlendMode(SkCanvas *canvas)
{
    canvas->clear(0);
    SkPaint paint;
    paint.setColor(0xFF00FFFF);
    auto rect1 = SkRect::MakeLTRB(60, h / 2 - 30, w - 60, h / 2 + 30);
    canvas->drawRect(rect1, paint);
    paint.setColor(0xFFFFFF00);
    paint.setBlendMode(SkBlendMode::kSrcOut);
    auto rect2 = SkRect::MakeLTRB(w / 2 - 30, 60, w / 2 + 30, h - 60);
    canvas->drawRect(rect2, paint);
}

void drawEraser(SkCanvas *canvas)
{
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setColor(0xFF00FFFF);
    auto r = std::min(w / 2 - 60, h / 2 - 60);
    canvas->drawCircle(w / 2, h / 2, r, paint);
    paint.setBlendMode(SkBlendMode::kClear);
    canvas->drawRect(SkRect::MakeXYWH(w / 2 - 50, h / 2 - 50, 100, 100), paint);
}

void paint(const HWND hWnd)
{
    if (w <= 0 || h <= 0)
        return;
    SkColor *surfaceMemory = new SkColor[w * h]{0xff000000};
    SkImageInfo info = SkImageInfo::MakeN32Premul(w, h);
    auto canvas = SkCanvas::MakeRasterDirect(info, surfaceMemory, 4 * w);
     averageColor(canvas.get());
    // colorOverlay(canvas.get());
    // formatColor();
    // drawBlendMode(canvas.get());
    //drawEraser(canvas.get());

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