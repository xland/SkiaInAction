#include <windows.h>
#include <windowsx.h>
#include <string>
#include <format>
#include <memory>
#include "include/core/SkGraphics.h"
#include "include/core/SkCanvas.h"
#include "include/gpu/GrDirectContext.h"
#include "GLContext.h"


HWND hwnd;
int w{ 800 }, h{ 600 };
GLContext* context;

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
        return true;
    }
    case WM_PAINT: {
        HDC dc = BeginPaint(hWnd, &ps);
        auto surface = context->GetSurface();
        auto canvas = surface->getCanvas();
        SkPaint paint;
        paint.setColor(SK_ColorRED);
        paint.setStyle(SkPaint::kFill_Style);
        canvas->drawRect(SkRect::MakeXYWH(50, 50, 200, 100), paint);
        context->fContext->flushAndSubmit(surface.get(), GrSyncCpu::kNo);
        SwapBuffers(dc);
        ReleaseDC((HWND)hWnd, dc);
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
    context = new GLContext(hwnd);
    
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPTSTR lpCmdLine, _In_ int nCmdShow)
{
    SkGraphics::Init();
    initWindow();
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}