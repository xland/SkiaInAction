#include <windows.h>
#include <string>
#include "include/core/SkColorSpace.h"
#include "include/core/SkSurface.h"
#include "include/core/SkCanvas.h"

#include "gl/DisplayParams.h"
#include "gl/SkWGL.h"
#include "gl/WindowContext.h"
#include "gl/GLWindowContext.h"
#include "gl/GLTestContext.h"
#include "gl/WindowContextFactory_win.h"
#include "include/gpu/GrDirectContext.h"

#include <GL/gl.h>
#include <vector>
int w{400}, h{400};
std::unique_ptr<skwindow::WindowContext> fWindowContext;
void paint(HWND hwnd)
{
    if (w <= 0 || h <= 0 || !fWindowContext) {
        return;
    }
    PAINTSTRUCT ps;
    auto dc = BeginPaint(hwnd, &ps);
    auto surface = fWindowContext->getBackbufferSurface();
    if (!surface.get()) {
        return;
    }
    SkRect rect = SkRect::MakeXYWH(w - 200, h - 200, 180, 180);
    auto canvas = surface->getCanvas();
    SkPaint paint;
    paint.setColor(0xff00ffff);
    canvas->drawRect(rect,paint);

    if (auto dContext = fWindowContext->directContext()) {
        dContext->flushAndSubmit(surface.get(), GrSyncCpu::kNo);
    }    
    fWindowContext->swapBuffers();

    EndPaint(hwnd, &ps);
}

LRESULT CALLBACK wndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_PAINT:
    {
        paint(hWnd);
        return 0;
    }
    case WM_SIZE:
    {
        w = LOWORD(lParam);
        h = HIWORD(lParam);
        if (!fWindowContext) {
            return 0;
        }
        fWindowContext->resize(w, h);
        return 0;
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
    fWindowContext = skwindow::MakeGLForWin(hwnd, skwindow::DisplayParams());
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