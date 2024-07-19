#include <windows.h>
#include <string>
#include "include/core/SkSurface.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkRRect.h"
#include "include/core/SkPath.h"


#include "include/core/SkFontMgr.h"
#include "include/core/SkFontStyle.h"
#include "include/ports/SkTypeface_win.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontMetrics.h"

int w{400}, h{400}; 

void drawText(SkCanvas *canvas)
{
    auto fontMgr = SkFontMgr_New_GDI();
    SkFontStyle fontStyle = SkFontStyle::Normal();
    auto typeFace = fontMgr->matchFamilyStyle("Microsoft YaHei", fontStyle);
    SkFont font(typeFace,56);

    SkPaint paint;
    paint.setColor(0xFF00FFFF);

    canvas->drawString("Hello World!", 20, 120, font, paint);
}


void drawCJKText(SkCanvas* canvas)
{
    SkPaint paint;
    //paint.setColor(0xFFFFFF00);
    //paint.setStroke(true);
    //paint.setStrokeWidth(1);
    //canvas->drawLine(SkPoint::Make(20, 0), SkPoint::Make(20, h), paint);
    //canvas->drawLine(SkPoint::Make(0, 120), SkPoint::Make(w, 120), paint);

    auto fontMgr = SkFontMgr_New_GDI();
    auto fontStyle = SkFontStyle::Normal();
    auto typeFace = fontMgr->matchFamilyStyle("Microsoft YaHei", fontStyle);
    SkFont font(typeFace, 56);

    std::wstring text{ L"你好，世界！" };
    auto length = text.size() * sizeof(wchar_t);

    paint.setColor(0xFF00FFFF);
    paint.setStroke(false);
    canvas->drawSimpleText(text.c_str(), length, SkTextEncoding::kUTF16, 20, 120, font, paint);
}

void measureText(SkCanvas* canvas)
{
    SkPaint paint;
    paint.setColor(0xFFFFFF00);
    paint.setStroke(true);
    paint.setStrokeWidth(1);
    canvas->drawLine(SkPoint::Make(w / 2, 0), SkPoint::Make(w / 2, h), paint);
    canvas->drawLine(SkPoint::Make(0, h / 2), SkPoint::Make(w, h / 2), paint);

    auto fontMgr = SkFontMgr_New_GDI();
    auto fontStyle = SkFontStyle::Normal();
    auto typeFace = fontMgr->matchFamilyStyle("Microsoft YaHei", fontStyle);
    SkFont font(typeFace, 56);

    std::wstring text{ L"你好，世界！" };
    auto length = text.size() * sizeof(wchar_t);

    SkRect measureRect;
    font.measureText(text.c_str(), length, SkTextEncoding::kUTF16, &measureRect);
    auto x = w / 2 - measureRect.width() / 2 - measureRect.fLeft;
    auto y = h / 2 - measureRect.height() / 2 - measureRect.fTop;

    //auto x = 0 - measureRect.fLeft;
    //auto y = 0 - measureRect.fTop;

    paint.setColor(0xFF00FFFF);
    paint.setStroke(false);

    canvas->drawSimpleText(text.c_str(), length, SkTextEncoding::kUTF16, x, y, font, paint);
}


void paint(const HWND hWnd)
{
    if (w <= 0 || h <= 0)
        return;
    SkColor *surfaceMemory = new SkColor[w * h]{0xff000000};
    SkImageInfo info = SkImageInfo::MakeN32Premul(w, h);
    auto canvas = SkCanvas::MakeRasterDirect(info, surfaceMemory, 4 * w);
    //drawText(canvas.get());
    //drawCJKText(canvas.get());
    measureText(canvas.get());


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
    auto hwnd = CreateWindow(clsName.c_str(), clsName.c_str(), WS_OVERLAPPEDWINDOW,
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