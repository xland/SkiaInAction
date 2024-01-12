#include <windows.h>
#include <windowsx.h>
#include <string>
#include <format>
#include "include/core/SkGraphics.h"
#include "include/core/SkSurface.h"
#include "include/core/SkCanvas.h"
#include "include/gpu/GrDirectContext.h"
#include "src/base/SkAutoMalloc.h"
#include "memory"

HWND hwnd;
sk_sp<SkSurface> surface;
int w{ 800 }, h{ 600 };
SkAutoMalloc surfaceMemory;

void initSurface() {
    surface.reset();
    const size_t bmpSize = sizeof(BITMAPINFOHEADER) + w * h * sizeof(uint32_t);
    surfaceMemory.reset(bmpSize);
    BITMAPINFO* bmpInfo = reinterpret_cast<BITMAPINFO*>(surfaceMemory.get());
    ZeroMemory(bmpInfo, sizeof(BITMAPINFO));
    bmpInfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmpInfo->bmiHeader.biWidth = w;
    bmpInfo->bmiHeader.biHeight = -h; // negative means top-down bitmap. Skia draws top-down.
    bmpInfo->bmiHeader.biPlanes = 1;
    bmpInfo->bmiHeader.biBitCount = 32;
    bmpInfo->bmiHeader.biCompression = BI_RGB;
    void* pixels = bmpInfo->bmiColors;
    SkImageInfo info = SkImageInfo::MakeN32Premul(w, h);
    surface = SkSurfaces::WrapPixels(info, pixels, sizeof(uint32_t) * w);
}

LRESULT CALLBACK wndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;
    switch (message) {
    case WM_CLOSE:{
        PostQuitMessage(0);
        return true;
    }
    case WM_SIZE: {
        w = LOWORD(lParam);
        h = HIWORD(lParam);
        initSurface();
        return true;
    }
    case WM_PAINT: {
        BeginPaint(hWnd, &ps);

        SkCanvas* canvas = surface->getCanvas();
        SkPaint paint;
        paint.setColor(SK_ColorRED);
        paint.setStyle(SkPaint::kFill_Style);
        canvas->drawRect(SkRect::MakeXYWH(50, 50, 200, 100), paint);

        BITMAPINFO* bmpInfo = reinterpret_cast<BITMAPINFO*>(surfaceMemory.get());
        HDC dc = GetDC(hWnd);
        StretchDIBits(dc, 0, 0, w, h, 0, 0, w, h, bmpInfo->bmiColors, bmpInfo,DIB_RGB_COLORS, SRCCOPY);
        ReleaseDC(hWnd, dc);

        EndPaint(hWnd, &ps);
        return true;
    }
    default:
        break;
    }
    return DefWindowProc(hWnd, message, wParam, lParam);
}

void initWindow() {
    std::wstring clsName{ L"SkiaInAction" };
    auto hinstance = GetModuleHandle(NULL);
    WNDCLASSEX wcx{};
    wcx.cbSize = sizeof(WNDCLASSEX);
    wcx.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS | CS_OWNDC;
    wcx.lpfnWndProc = wndProc;
    wcx.cbClsExtra = 0;
    wcx.cbWndExtra = 0;
    wcx.hInstance = hinstance;
    wcx.hIcon = LoadIcon(hinstance, (LPCTSTR)IDI_WINLOGO);
    wcx.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcx.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcx.lpszMenuName = nullptr;
    wcx.lpszClassName = clsName.c_str();
    wcx.hIconSm = LoadIcon(hinstance, (LPCTSTR)IDI_WINLOGO);
    if (!RegisterClassEx(&wcx)) {
        return;
    }
    hwnd = CreateWindow(clsName.c_str(), nullptr, WS_OVERLAPPEDWINDOW,100, 100, w, h,nullptr, nullptr, hinstance, nullptr);
    ShowWindow(hwnd, SW_SHOW);
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPTSTR lpCmdLine, _In_ int nCmdShow)
{
    SkGraphics::Init();
    initSurface();
    initWindow();
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}