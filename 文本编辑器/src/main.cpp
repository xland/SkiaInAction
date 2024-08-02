#include <windows.h>
#include <string>
#include "include/core/SkSurface.h"
#include "include/core/SkCanvas.h"

#include "include/core/SkFontMgr.h"
#include "include/ports/SkTypeface_win.h"

#include "editor/editor.h"
#include "editor/stringslice.h"
#include "editor/stringview.h"

#define WM_REFRESH (WM_APP+100)
SkColor* surfaceMemory;
HWND hwnd;
int w{800}, h{800};
int fMargin = 10;
SkPlainTextEditor::Editor fEditor;
SkPlainTextEditor::Editor::TextPosition fTextPos{ 0, 0 };
SkPlainTextEditor::Editor::TextPosition fMarkPos;
bool fBlink = false;

std::string wideStrToStr(const std::wstring& wstr)
{
    const int count = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.length(), NULL, 0, NULL, NULL);
    std::string str(count, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &str[0], count, NULL, NULL);
    return str;
}


void init() {
    surfaceMemory = new SkColor[w * h]{ 0xff000000 };

    std::wstring text{ L"你好" };
    auto text2 = wideStrToStr(text);
    fEditor.insert(SkPlainTextEditor::Editor::TextPosition{ 0, 0 }, text2.data(), text2.length());

    auto fontMgr = SkFontMgr_New_GDI();
    fEditor.setFontMgr(fontMgr);
    SkFontStyle style(SkFontStyle::kNormal_Weight, SkFontStyle::kNormal_Width, SkFontStyle::kUpright_Slant);
    auto typeFace = fontMgr->matchFamilyStyle("Microsoft YaHei", style);
    fEditor.setFont(SkFont(typeFace, 32));
    fEditor.setWidth(w - 2 * fMargin);
    SetTimer(hwnd, WM_REFRESH, 600, (TIMERPROC)NULL);
}
void repaintWin(const HWND hWnd)
{
    if (w <= 0 || h <= 0)
        return;
    SkImageInfo info = SkImageInfo::MakeN32Premul(w, h);
    auto canvas = SkCanvas::MakeRasterDirect(info, surfaceMemory, 4 * w);
    canvas->clear(0xFFFFFFFF);
    SkPlainTextEditor::Editor::PaintOpts options;
    options.fCursor = fTextPos;
    options.fBackgroundColor = SkColor4f{ 0.8f, 0.8f, 0.8f, 1 };
    options.fCursorColor = { 1, 0, 0, fBlink ? 0.0f : 1.0f };
    if (fMarkPos != SkPlainTextEditor::Editor::TextPosition()) {
        options.fSelectionBegin = fMarkPos;
        options.fSelectionEnd = fTextPos;
    }
    canvas->translate(fMargin,fMargin);
    fEditor.paint(canvas.get(), options);

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
        case WM_TIMER: {
            switch (wParam)
            {
                case WM_REFRESH: {
                    fBlink = !fBlink;
                    InvalidateRect(hWnd, nullptr, false);
                    break;
                }
                default:
                {
                    break;
                }
            }
            return 0;
        }
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
            repaintWin(hWnd);
            break;
        }
        case WM_CLOSE:
        {
            PostQuitMessage(0);
            break;
        }
        case WM_KEYDOWN: {
            switch (wParam)
            {
                case VK_LEFT:
                {
                    auto pos = fEditor.move(SkPlainTextEditor::Editor::Movement::kLeft, fTextPos);
                    InvalidateRect(hWnd, nullptr, false);
                    break;
                }
                case VK_RIGHT:
                {
                    auto pos = fEditor.move(SkPlainTextEditor::Editor::Movement::kRight, fTextPos);
                    if (pos == fTextPos || pos == SkPlainTextEditor::Editor::TextPosition()) {
                        fMarkPos = SkPlainTextEditor::Editor::TextPosition();
                    }
                    fMarkPos = SkPlainTextEditor::Editor::TextPosition();
                    fTextPos = pos;
                    InvalidateRect(hWnd, nullptr, false);
                    break;
                }
                default:
                {
                    break;
                }
            }
            return 0;
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
    init();
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
    delete[] surfaceMemory;
    return 0;
}