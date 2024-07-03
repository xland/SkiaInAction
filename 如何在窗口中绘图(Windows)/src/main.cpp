#include <windows.h>
#include <windowsx.h>
#include <string>
#include <format>
#include <memory>

#include "include/core/SkGraphics.h"
#include "include/core/SkSurface.h"
#include "include/core/SkCanvas.h"
#include "src/base/SkAutoMalloc.h"

HWND hwnd;
sk_sp<SkSurface> surface;
int w{ 800 }, h{ 600 };
SkAutoMalloc surfaceMemory;

void initSurface() {
    surface.reset();
    surfaceMemory.reset(h * 4 * w);
    SkImageInfo info = SkImageInfo::MakeN32Premul(w, h);
    surface = SkSurfaces::WrapPixels(info, surfaceMemory.get(), 4 * w);

    SkCanvas* canvas = surface->getCanvas();
    canvas->clear(SK_ColorBLACK);
    SkPaint paint;
    paint.setColor(SK_ColorRED);
    canvas->drawRect(SkRect::MakeLTRB(w - 150, h - 150, w - 10, h - 10), paint);
}

LRESULT CALLBACK wndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;
    switch (message) {
    case WM_CLOSE: {
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
        auto dc = BeginPaint(hWnd, &ps); 
        BITMAPINFO info = { sizeof(BITMAPINFOHEADER), w, 0 - h, 1, 32, BI_RGB, h * 4 * w, 0, 0, 0, 0 };
        StretchDIBits(dc, 0, 0, w, h, 0, 0, w, h, surfaceMemory.get(), &info, DIB_RGB_COLORS, SRCCOPY);
        ReleaseDC(hWnd, dc);
        EndPaint(hWnd, &ps);
        surfaceMemory.reset(0);
        return true;
    }
    default:
        break;
    }
    return DefWindowProc(hWnd, message, wParam, lParam);
}

void initWindow() {
    std::wstring clsName{ L"DrawInWindow" };
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
    hwnd = CreateWindow(clsName.c_str(), clsName.c_str(), WS_OVERLAPPEDWINDOW, 100, 100, w, h, nullptr, nullptr, hinstance, nullptr);
    ShowWindow(hwnd, SW_SHOW);
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPTSTR lpCmdLine, _In_ int nCmdShow)
{
    initSurface();
    initWindow();
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    surfaceMemory.release();
    return 0;
}