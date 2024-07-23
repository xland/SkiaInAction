#include <windows.h>
#include <string>
#include "include/core/SkSurface.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkRRect.h"
#include "include/core/SkPath.h"

#include "include/core/SkStream.h"
#include "include/core/SkBitmap.h"
#include "include/codec/SkCodec.h"

#include "include/core/SkMaskFilter.h"
#include "include/core/SkBlurTypes.h"
#include "include/core/SkColorFilter.h"
#include "include/effects/SkImageFilters.h"

#include "include/encode/SkPngEncoder.h"
#include "include/encode/SkJpegEncoder.h"
#include "include/encode/SkWebpEncoder.h"

#include <thread>

int w{400}, h{400};
HWND hwnd;
std::string wideStrToStr(const std::wstring& wstr)
{
    const int count = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.length(), NULL, 0, NULL, NULL);
    std::string str(count, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &str[0], count, NULL, NULL);
    return str;
}


void animateGif(SkCanvas *canvas)
{
    canvas->clear(0xFFFFFFFF);
    std::wstring imgPath = L"D:\\project\\SkiaInAction\\动画\\demo.gif";
    auto pathStr = wideStrToStr(imgPath);
    std::unique_ptr<SkFILEStream> stream = SkFILEStream::Make(pathStr.data());
    std::unique_ptr<SkCodec> codec = SkCodec::MakeFromStream(std::move(stream));
    auto imgInfo = codec->getInfo();
    SkColor* frameMem = new SkColor[imgInfo.width() * imgInfo.height()]();
    auto t = std::thread([&](std::unique_ptr<SkCodec> codec) {
        auto frameCount = codec->getFrameCount();
        auto frameInfo = codec->getFrameInfo();
        auto imgInfo = codec->getInfo();
        SkCodec::Options option;
        option.fFrameIndex = 0;
        option.fPriorFrame = -1;
        while (true)
        {
            auto start = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
            codec->getPixels(imgInfo, frameMem, imgInfo.minRowBytes(), &option);
            canvas->writePixels(imgInfo, frameMem, imgInfo.width() * 4, 0, 0);
            InvalidateRect(hwnd, nullptr, false);
            auto end = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
            auto span = frameInfo[option.fFrameIndex].fDuration - (end - start);
            std::this_thread::sleep_for(std::chrono::milliseconds(span));
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
    SkColor *surfaceMemory = new SkColor[w * h]{0xff000000};
    SkImageInfo info = SkImageInfo::MakeN32Premul(w, h);
    auto canvas = SkCanvas::MakeRasterDirect(info, surfaceMemory, 4 * w);
    animateGif(canvas.get());

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
    hwnd = CreateWindow(clsName.c_str(), clsName.c_str(), WS_OVERLAPPEDWINDOW,
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