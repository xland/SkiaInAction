#include <windows.h>
#include <string>
#include "include/core/SkSurface.h"
#include "include/core/SkCanvas.h"
#include "include/effects/SkGradientShader.h"
#include "include/core/SkPath.h"
#include "include/core/SkPicture.h"
#include "include/core/SkPictureRecorder.h"
#include "src/base/SkRandom.h"
#include "include/core/SkStream.h"

#include <vector>

int w{3840}, h{2160};

std::vector<uint32_t> rs;
std::vector<uint32_t> xs;
std::vector<uint32_t> ys;
std::vector<uint32_t> cs;

void prepareData() {
    auto maxR = std::min(w / 2, h / 2);
    SkRandom rnd;
    for (size_t i = 0; i < 10000; i++)
    {
        auto r = rnd.nextRangeU(10, maxR);
        auto x = rnd.nextRangeU(0 + r, w - r);
        auto y = rnd.nextRangeU(0 + r, h - r);
        auto color = rnd.nextRangeU(0xFF666666, 0xFFFFFFFF);
        rs.push_back(r);
        xs.push_back(x);
        ys.push_back(y);
        cs.push_back(color);
    }
}


void drawCircles(SkCanvas* canvas) {

    for (size_t i = 0; i < 10000; i++)
    {
        SkPaint paint;
        paint.setColor(cs[i]);
        canvas->drawCircle(SkPoint::Make(xs[i], ys[i]), rs[i], paint);
    }
}


void recordCanvas(SkCanvas *canvas)
{
    SkPictureRecorder recorder;
    SkCanvas *canvasRecorder = recorder.beginRecording(w, h);
    for (size_t i = 0; i < 10000; i++)
    {
        SkPaint paint;
        paint.setColor(cs[i]);
        canvasRecorder->drawCircle(SkPoint::Make(xs[i], ys[i]), rs[i], paint);
    }
    sk_sp<SkPicture> picture = recorder.finishRecordingAsPicture();
    canvas->drawPicture(picture); //1.4秒
}

void paint(const HWND hWnd)
{
    if (w <= 0 || h <= 0)
        return;
    SkColor *surfaceMemory = new SkColor[w * h]{0xff000000};
    //SkColor* surfaceMemory2 = new SkColor[w * h]{ 0xff0000ff };
    SkImageInfo info = SkImageInfo::MakeN32Premul(w, h);
    auto canvas = SkCanvas::MakeRasterDirect(info, surfaceMemory, 4 * w);
    //auto canvas2 = SkCanvas::MakeRasterDirect(info, surfaceMemory2, 4 * w);
    //canvas->writePixels(info, surfaceMemory2, w * 4, 0, 0); //非常快 1毫秒

    prepareData();
    //drawCircles(canvas.get()); // 0.6秒

    recordCanvas(canvas.get());

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