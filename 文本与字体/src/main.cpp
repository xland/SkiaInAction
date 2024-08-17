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
#include "include/core/SkTextBlob.h"
#include "include/core/SkData.h"

int w{400}, h{400}; 

void drawText(SkCanvas *canvas)
{
    sk_sp<SkFontMgr> fontMgr = SkFontMgr_New_GDI();
    SkFontStyle fontStyle = SkFontStyle::Normal();
    sk_sp<SkTypeface> typeFace = fontMgr->matchFamilyStyle("Microsoft YaHei", fontStyle);
    SkFont font(typeFace,56);
    SkPaint paint;
    paint.setColor(0xFF00FFFF);

    canvas->drawString("Hello World!", 20, 120, font, paint);
}

std::string wideStrToStr(const std::wstring& wstr)
{
    const int count = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.length(), NULL, 0, NULL, NULL);
    std::string str(count, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &str[0], count, NULL, NULL);
    return str;
}

void drawCJKText(SkCanvas* canvas)
{
    SkPaint paint;
    paint.setColor(0xFF00FFFF);
    paint.setStroke(false);
    auto fontMgr = SkFontMgr_New_GDI();
    auto fontStyle = SkFontStyle::Normal();
    auto typeFace = fontMgr->matchFamilyStyle("Microsoft YaHei", fontStyle);
    SkFont font(typeFace, 38);

    std::wstring text1{ L"你好，世界！" };
    auto length = text1.size() * sizeof(wchar_t);    
    canvas->drawSimpleText(text1.c_str(), length, SkTextEncoding::kUTF16, 20, 120, font, paint);

    auto text2 = wideStrToStr(text1);
    canvas->drawString(text2.c_str(), 20, 240, font, paint);
    //canvas->drawSimpleText(text2.c_str(), text2.size(), SkTextEncoding::kUTF8, 20, 240, font, paint);

    std::u16string text3{ u"你好，世界！" };
    length = text3.size() * sizeof(char16_t);
    canvas->drawSimpleText(text3.c_str(), length, SkTextEncoding::kUTF16, 20, 180, font, paint);
}

void textPosition(SkCanvas* canvas)
{
    SkPaint paint;
    paint.setColor(0xFFFFFF00);
    paint.setStroke(true);
    paint.setStrokeWidth(1);
    canvas->drawLine(SkPoint::Make(20, 0), SkPoint::Make(20, h), paint);
    canvas->drawLine(SkPoint::Make(0, 120), SkPoint::Make(w, 120), paint);

    auto fontMgr = SkFontMgr_New_GDI();
    auto fontStyle = SkFontStyle::Normal();
    auto typeFace = fontMgr->matchFamilyStyle("Microsoft YaHei", fontStyle);
    SkFont font(typeFace, 56);

    std::wstring wideStr{ L"你好，世界！" };
    auto text = wideStrToStr(wideStr);

    paint.setColor(0xFF00FFFF);
    paint.setStroke(false);
    canvas->drawString(text.c_str(), 20, 120, font, paint);
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



void loadFontFile(SkCanvas* canvas) {
    auto fontPath = L"D:\\project\\SkiaInAction\\文本与字体\\AlimamaDaoLiTi.ttf";
    auto fontPathStr = wideStrToStr(fontPath);
    auto data{ SkData::MakeFromFileName(fontPathStr.data())};
    auto fontMgr = SkFontMgr_New_GDI();
    auto typeFace = fontMgr->makeFromData(data);
    SkFont font(typeFace,66);
    std::wstring text{ L"天地玄黄，宇宙洪荒！" };
    auto length = text.size() * sizeof(wchar_t);
    SkPaint paint;
    paint.setColor(0xFFFFFF00);
    paint.setStroke(false);
    canvas->drawSimpleText(text.c_str(), length, SkTextEncoding::kUTF16, 20, 120, font, paint);
}

void drawFontIcon(SkCanvas* canvas) {
    auto fontPath = L"D:\\project\\SkiaInAction\\文本与字体\\fa-solid-900.ttf";
    auto fontPathStr = wideStrToStr(fontPath);
    auto data{ SkData::MakeFromFileName(fontPathStr.data()) };
    auto fontMgr = SkFontMgr_New_GDI();
    auto typeFace = fontMgr->makeFromData(data);
    SkFont font(typeFace, 66);

    
    SkPaint paint;
    paint.setColor(0xFFFFFF00);
    paint.setStroke(false);

    auto iconCode = (const char*)u8"\uf015";
    canvas->drawString(iconCode, 20, 120, font, paint);

    iconCode = (const char*)u8"\uf118";
    canvas->drawString(iconCode, 120, 120, font, paint);

    iconCode = (const char*)u8"\uf06b";
    canvas->drawString(iconCode, 220, 120, font, paint);

    iconCode = (const char*)u8"\uf21d";
    canvas->drawString(iconCode, 320, 120, font, paint);

    iconCode = (const char*)u8"\uf086";
    canvas->drawString(iconCode, 420, 120, font, paint);

    iconCode = (const char*)u8"\uf53f";
    canvas->drawString(iconCode, 520, 120, font, paint);
}

void fontBorder(SkCanvas* canvas) {
    auto fontPath = L"D:\\project\\SkiaInAction\\文本与字体\\AlimamaDaoLiTi.ttf";
    auto fontPathStr = wideStrToStr(fontPath);
    auto data{ SkData::MakeFromFileName(fontPathStr.data()) };
    auto fontMgr = SkFontMgr_New_GDI();
    auto typeFace = fontMgr->makeFromData(data);
    SkFont font(typeFace, 86);
    font.setEdging(SkFont::Edging::kSubpixelAntiAlias);
    font.setSubpixel(true);
    std::wstring text{ L"天地玄黄，宇宙洪荒！" };
    auto length = text.size() * sizeof(wchar_t);
    SkPaint paint;
    paint.setColor(0xFFFFFF00);
    paint.setStroke(true);
    paint.setStrokeWidth(2);
    canvas->drawSimpleText(text.c_str(), length, SkTextEncoding::kUTF16, 20, 120, font, paint);
}

void textBlob(SkCanvas* canvas) {  
    auto fontPath = L"D:\\project\\SkiaInAction\\文本与字体\\AlimamaDaoLiTi.ttf";
    auto fontPathStr = wideStrToStr(fontPath);
    auto data{ SkData::MakeFromFileName(fontPathStr.data()) };
    auto fontMgr = SkFontMgr_New_GDI();
    auto typeFace = fontMgr->makeFromData(data);
    SkFont daoli(typeFace, 66);

    auto typeFace2 = fontMgr->matchFamilyStyle("Microsoft YaHei", SkFontStyle::Normal());
    SkFont yahei(typeFace2, 66);

    std::wstring str1 = L"天地玄黄，";
    auto length1 = str1.size() * sizeof(wchar_t);
    SkPaint paint;
    paint.setColor(0xFFFFFF00);
    sk_sp<SkTextBlob> blob1 =
        SkTextBlob::MakeFromText(str1.data(),length1,daoli, SkTextEncoding::kUTF16);

    std::wstring str11 = L"宇宙洪荒！";
    auto str2 = wideStrToStr(str11);
    sk_sp<SkTextBlob> blob2 =
        SkTextBlob::MakeFromString(str2.data(), yahei, SkTextEncoding::kUTF8);

    // 在画布上绘制 SkTextBlob
    canvas->drawTextBlob(blob1, 20, 120, paint);
    canvas->drawTextBlob(blob2, 20, 220, paint);
}

void paint(const HWND hWnd)
{
    if (w <= 0 || h <= 0)
        return;
    SkColor *surfaceMemory = new SkColor[w * h]{0xff000000};
    SkImageInfo info = SkImageInfo::MakeN32Premul(w, h);
    auto canvas = SkCanvas::MakeRasterDirect(info, surfaceMemory, 4 * w);
    drawText(canvas.get());
    //drawCJKText(canvas.get());
    //textPosition(canvas.get());
    //measureText(canvas.get());
    //loadFontFile(canvas.get());
    //drawFontIcon(canvas.get());
    //fontBorder(canvas.get());
    //textBlob(canvas.get());


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