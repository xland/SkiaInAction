#include <windows.h>
#include <string>
#include "include/core/SkSurface.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkStream.h"
#include "include/core/SkBitmap.h"
#include "include/codec/SkCodec.h"

#include "include/codec/SkEncodedImageFormat.h"

#include "modules/skottie/include/Skottie.h"
#include <thread>
#include <mutex>


int w{400}, h{400};
HWND hwnd;
SkColor* surfaceMemory{nullptr};
std::mutex locker;

std::string wideStrToStr(const std::wstring& wstr)
{
    const int count = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.length(), NULL, 0, NULL, NULL);
    std::string str(count, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &str[0], count, NULL, NULL);
    return str;
}

SkCanvas* getCanvas() {
    static std::unique_ptr<SkCanvas> canvas;
    if (!canvas.get()) {
        SkImageInfo info = SkImageInfo::MakeN32Premul(w, h);
        canvas = SkCanvas::MakeRasterDirect(info, surfaceMemory, 4 * w);
        return canvas.get();
    }
    auto info = canvas->imageInfo();
    if (w != info.width() || h != info.height()) {
        SkImageInfo newInfo = SkImageInfo::MakeN32Premul(w, h);
        canvas = SkCanvas::MakeRasterDirect(newInfo, surfaceMemory, 4 * w);
    }
    return canvas.get();
}

void animateLottie() {
    auto t = std::thread([&]() {
        std::wstring imgPath = L"D:\\project\\SkiaInAction\\动画Lottie\\demo.json";
        auto pathStr = wideStrToStr(imgPath);
        sk_sp<skottie::Animation> animation = skottie::Animation::MakeFromFile(pathStr.data());
        auto size = animation->size();
        auto fps = animation->fps();
        auto duration = std::chrono::milliseconds((int)(1000 / fps));
        SkScalar frameCount = animation->duration()*fps;
        auto frameIndex{ 0 };
        while (true)
        {
            auto x = (w - size.fWidth) / 2;
            auto y = (h - size.fHeight) / 2;
            auto rect = SkRect::MakeXYWH(x, y, size.fWidth, size.fHeight);
            animation->seekFrame(frameIndex);
            std::unique_lock guard(locker);
            auto canvas = getCanvas();
            canvas->clear(0xff000000);
            animation->render(canvas, &rect);
            guard.unlock();
            InvalidateRect(hwnd, nullptr, false);
            std::this_thread::sleep_for(duration);
            frameIndex += 1;
            if (frameIndex > frameCount) {
                frameIndex = 0;
            }
        }
    });
    t.detach();    
}

void paint(const HWND hWnd)
{
    if (w <= 0 || h <= 0)
        return;
    PAINTSTRUCT ps;
    auto dc = BeginPaint(hWnd, &ps);
    BITMAPINFO bmpInfo = {sizeof(BITMAPINFOHEADER), w, 0 - h, 1, 32, BI_RGB, h * 4 * w, 0, 0, 0, 0};
    StretchDIBits(dc, 0, 0, w, h, 0, 0, w, h, surfaceMemory, &bmpInfo, DIB_RGB_COLORS, SRCCOPY);
    ReleaseDC(hWnd, dc);
    EndPaint(hWnd, &ps);
}

LRESULT CALLBACK wndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_SIZE:
    {
        w = LOWORD(lParam);
        h = HIWORD(lParam);
        if (surfaceMemory) {
            std::unique_lock guard(locker);
            delete[] surfaceMemory;
            surfaceMemory = new SkColor[w * h]{ 0xff000000 };
            guard.unlock();
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
    hwnd = CreateWindow(clsName.c_str(), clsName.c_str(), WS_OVERLAPPEDWINDOW,
                             CW_USEDEFAULT, CW_USEDEFAULT, w, h,
                             nullptr, nullptr, hinstance, nullptr);
    ShowWindow(hwnd, SW_SHOW);
    surfaceMemory = new SkColor[w * h]{ 0xff000000 };
    animateLottie();
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