#include <windows.h>
#include <windowsx.h>
#include <string>
#include <format>
#include "include/core/SkGraphics.h"


void initWindow() {
    std::wstring clsName{ L"SkiaInAction" };
    auto hinstance = GetModuleHandle(NULL);
    WNDCLASSEX wcx{};
    wcx.cbSize = sizeof(WNDCLASSEX);
    wcx.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS | CS_OWNDC;
    //wcx.lpfnWndProc = WndProc;
    wcx.cbClsExtra = 0;
    wcx.cbWndExtra = 0;
    wcx.hInstance = hinstance;
    wcx.hIcon = LoadIcon(hinstance, (LPCTSTR)IDI_WINLOGO);
    wcx.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcx.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcx.lpszMenuName = nullptr;
    wcx.lpszClassName = clsName.c_str();
    wcx.hIconSm = LoadIcon(hinstance, (LPCTSTR)IDI_WINLOGO);
    if (!RegisterClassEx(&wcx))
    {
        return;
    }
    auto hwnd = CreateWindow(clsName.c_str(), nullptr, WS_OVERLAPPEDWINDOW,100, 100, 800, 600,
        nullptr, nullptr, hinstance, nullptr);;
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPTSTR lpCmdLine, _In_ int nCmdShow)
{
    SkGraphics::Init();
    initWindow();
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}