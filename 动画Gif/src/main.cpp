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


int w{400}, h{400};
HWND hwnd;
SkColor* surfaceMemory{nullptr};
SkBitmap* frameBitmap;

std::string wideStrToStr(const std::wstring& wstr)
{
    const int count = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.length(), NULL, 0, NULL, NULL);
    std::string str(count, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &str[0], count, NULL, NULL);
    return str;
}
void animateGif()
{
    std::wstring imgPath = L"D:\\project\\SkiaInAction\\动画Gif\\demo.gif";
    auto pathStr = wideStrToStr(imgPath);
    std::unique_ptr<SkFILEStream> stream = SkFILEStream::Make(pathStr.data());
    std::unique_ptr<SkCodec> codec = SkCodec::MakeFromStream(std::move(stream));
    frameBitmap = new SkBitmap();
    auto t = std::thread([](std::unique_ptr<SkCodec> codec) {
        auto imgInfo = codec->getInfo().makeColorType(kN32_SkColorType);
        frameBitmap->allocN32Pixels(imgInfo.width(), imgInfo.height());
        int frameCount = codec->getFrameCount();
        std::vector<SkCodec::FrameInfo> frameInfo = codec->getFrameInfo();
        SkCodec::Options option;
        option.fFrameIndex = 0;
        option.fPriorFrame = -1;
        while (true)
        {
            auto start = std::chrono::system_clock::now();
            codec->getPixels(imgInfo, frameBitmap->getPixels(), imgInfo.minRowBytes(), &option);
            InvalidateRect(hwnd, nullptr, false);
            auto end = std::chrono::system_clock::now();
            auto tSpan = end - start;
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(tSpan);
            auto msCount = frameInfo[option.fFrameIndex].fDuration - ms.count();
            auto duration = std::chrono::milliseconds(msCount);
            std::this_thread::sleep_for(duration);
            if (option.fFrameIndex == frameCount - 1)
            {
                option.fPriorFrame = -1;
                option.fFrameIndex = 0;
            }
            else
            {
                option.fPriorFrame = option.fPriorFrame + 1;
                option.fFrameIndex = option.fFrameIndex + 1;
            }
        }
        }, std::move(codec));
    t.detach();
}

void paint(const HWND hWnd)
{
    if (w <= 0 || h <= 0)
        return;  
    SkImageInfo info = SkImageInfo::MakeN32Premul(w, h);
    auto canvas = SkCanvas::MakeRasterDirect(info, surfaceMemory, 4 * w);
    if (frameBitmap) {
        auto x = (w - frameBitmap->width()) / 2;
        auto y = (h - frameBitmap->height()) / 2;
        canvas->writePixels(*frameBitmap, x, y);
    }
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
            delete[] surfaceMemory;
            surfaceMemory = new SkColor[w * h]{ 0xff000000 };
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
    animateGif();
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