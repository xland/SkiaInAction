#include <windows.h>
#include <string>
#include "include/core/SkSurface.h"
#include "include/core/SkCanvas.h"

#include "include/core/SkFontMgr.h"
#include "include/ports/SkTypeface_win.h"
#include "src/base/SkBitmaskEnum.h"
#include "src/base/SkUTF.h"

#include "editor/editor.h"
#include "editor/stringslice.h"
#include "editor/stringview.h"
using Editor = SkPlainTextEditor::Editor;


#define WM_REFRESH (WM_APP+100)
SkColor* surfaceMemory;
HWND hwnd;
int w{800}, h{800};
int fMargin = 10;
int fPos = 0;
Editor fEditor;
Editor::TextPosition fTextPos{ 0, 0 };
Editor::TextPosition fMarkPos;
bool fShiftDown = false;
bool fBlink = false;

void activeKeyboard(long x,long y) {
    if (HIMC himc = ImmGetContext(hwnd))
    {
        COMPOSITIONFORM comp = {};
        comp.ptCurrentPos.x = x;
        comp.ptCurrentPos.y = y;
        comp.dwStyle = CFS_FORCE_POSITION;
        ImmSetCompositionWindow(himc, &comp);
        CANDIDATEFORM cand = {};
        cand.dwStyle = CFS_CANDIDATEPOS;
        cand.ptCurrentPos.x = x;
        cand.ptCurrentPos.y = y;
        ImmSetCandidateWindow(himc, &cand);
        ImmReleaseContext(hwnd, himc);
    }
}

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
    fEditor.insert(Editor::TextPosition{ 0, 0 }, text2.data(), text2.length());
    auto fontMgr = SkFontMgr_New_GDI();
    fEditor.setFontMgr(fontMgr);
    SkFontStyle style(SkFontStyle::kNormal_Weight, SkFontStyle::kNormal_Width, SkFontStyle::kUpright_Slant);
    auto typeFace = fontMgr->matchFamilyStyle("Microsoft YaHei", style);
    fEditor.setFont(SkFont(typeFace, 32));
    fEditor.setWidth(w - 2 * fMargin);

    SkIRect cursor = fEditor.getLocation(fTextPos).roundOut();
    activeKeyboard(cursor.fRight, cursor.fBottom);

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

static Editor::Movement getMoveType(unsigned int key) {
    switch (key) {
    case VK_LEFT:  return Editor::Movement::kLeft;
    case VK_RIGHT: return Editor::Movement::kRight;
    case VK_UP:    return Editor::Movement::kUp;
    case VK_DOWN:  return Editor::Movement::kDown;
    case VK_HOME:  return Editor::Movement::kHome;
    case VK_END:   return Editor::Movement::kEnd;
    default: return Editor::Movement::kNowhere;
    }
}

bool move(Editor::TextPosition pos, bool shift) {
    if (pos == fTextPos || pos == Editor::TextPosition()) {
        if (!shift) {
            fMarkPos = Editor::TextPosition();
        }
        return false;
    }
    if (shift != fShiftDown) {
        fMarkPos = shift ? fTextPos : Editor::TextPosition();
        fShiftDown = shift;
    }
    fTextPos = pos;
    // scroll if needed.
    SkIRect cursor = fEditor.getLocation(fTextPos).roundOut();
    activeKeyboard(cursor.fRight, cursor.fBottom);


    if (fPos < cursor.bottom() - h + 2 * fMargin) {
        fPos = cursor.bottom() - h + 2 * fMargin;
    }
    else if (cursor.top() < fPos) {
        fPos = cursor.top();
    }
    fBlink = true;
    InvalidateRect(hwnd, nullptr, false);
    return true;
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
            bool shift = (GetAsyncKeyState(VK_LSHIFT) & 0x8000) || (GetAsyncKeyState(VK_RSHIFT) & 0x8000);
            switch (wParam)
            {
                case VK_LEFT:
                case VK_RIGHT:
                case VK_UP:
                case VK_DOWN:
                case VK_HOME:
                case VK_END:
                {
                    auto moveType = getMoveType(wParam);
                    auto pos = fEditor.move(moveType, fTextPos);
                    move(pos, shift);
                    break;
                }
                case VK_DELETE:
                {
                    if (fMarkPos != Editor::TextPosition()) {
                        auto textPos = fEditor.remove(fMarkPos, fTextPos);
                        move(textPos, false);
                    }
                    else {
                        auto pos = fEditor.move(Editor::Movement::kRight, fTextPos);
                        auto textPos = fEditor.remove(fTextPos, pos);
                        move(textPos, false);
                    }
                    break;
                }
                case VK_BACK: {
                    if (fMarkPos != Editor::TextPosition()) {
                        move(fEditor.remove(fMarkPos, fTextPos), false);
                    }
                    else {
                        auto pos = fEditor.move(Editor::Movement::kLeft, fTextPos);
                        move(fEditor.remove(fTextPos, pos), false);
                    }
                    break;
                }
                case VK_RETURN: {
                    char ch = (char)'\n';
                    fEditor.insert(fTextPos, &ch, 1);
                    auto moveType = getMoveType(VK_RIGHT);
                    auto pos = fEditor.move(moveType, fTextPos);
                    move(pos, false);
                    return 0;
                }
                default:
                {
                    break;
                }
            }
            return 0;
        }
        case WM_CHAR: {
            if ((wParam >= 32 && wParam <= 126) || // 可打印的ASCII字符范围
                (wParam >= 160 && wParam <= 55295) || // 可打印的Unicode字符范围
                (wParam >= 57344 && wParam <= 65535)) // 高位可打印的Unicode字符范围
            { 
                std::wstring word{ (wchar_t)wParam };
                auto text2 = wideStrToStr(word);
                fEditor.insert(fTextPos, text2.data(), text2.length());
                auto moveType = getMoveType(VK_RIGHT);
                auto pos = fEditor.move(moveType, fTextPos);
                move(pos, false);
                return 0;
            }
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