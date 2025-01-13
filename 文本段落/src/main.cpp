#include <windows.h>
#include <string>
#include "include/core/SkSurface.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkStream.h"


#include "include/core/SkFontMgr.h"
#include "include/ports/SkTypeface_win.h"
#include "modules/skparagraph/include/Paragraph.h"
#include "modules/skparagraph/src/ParagraphBuilderImpl.h"


int w{800}, h{800};

void drawMutiText(SkCanvas* canvas)
{
    std::u16string text(uR"(醉里挑灯看剑，梦回吹角连营。
八百里分麾下炙，五十弦翻塞外声。
沙场秋点兵。
马作的卢飞快，弓如霹雳弦惊。
了却君王天下事，赢得生前身后名。
可怜白发生！)");
    sk_sp<skia::textlayout::FontCollection> fontCollection = sk_make_sp<skia::textlayout::FontCollection>();
    auto fontMgr = SkFontMgr_New_GDI();
    fontCollection->setDefaultFontManager(fontMgr);
    skia::textlayout::ParagraphStyle paraStyle;
    std::unique_ptr<skia::textlayout::ParagraphBuilder> builder = skia::textlayout::ParagraphBuilder::make(paraStyle, fontCollection);
    skia::textlayout::TextStyle defaultStyle;
    defaultStyle.setFontFamilies({ SkString{"Microsoft YaHei"},SkString{"Segoe UI Emoji"} });
    defaultStyle.setColor(0xff00ffff);
    defaultStyle.setFontSize(38);
    builder->pushStyle(defaultStyle);
    builder->addText(text);
    std::unique_ptr<skia::textlayout::Paragraph> paragraph = builder->Build();
    paragraph->layout(w);
    paragraph->paint(canvas, 10, 10);
}

void drawMutiText2(SkCanvas *canvas)
{  
    sk_sp<skia::textlayout::FontCollection> fontCollection = sk_make_sp<skia::textlayout::FontCollection>();
    auto fontMgr = SkFontMgr_New_GDI();
    fontCollection->setDefaultFontManager(fontMgr);
    skia::textlayout::ParagraphStyle paraStyle;
    skia::textlayout::StrutStyle strutStyle;
    strutStyle.setFontFamilies({ SkString{"Microsoft YaHei"} });
    strutStyle.setStrutEnabled(true);
    strutStyle.setFontSize(38);
    strutStyle.setHeightOverride(true);
    strutStyle.setHeight(1.6);
    paraStyle.setStrutStyle(strutStyle);
    auto builder = skia::textlayout::ParagraphBuilder::make(paraStyle, fontCollection);

    skia::textlayout::TextStyle defaultStyle;
    defaultStyle.setFontFamilies({ SkString{"Microsoft YaHei"} });
    defaultStyle.setColor(0xff00ffff);
    defaultStyle.setFontSize(38);
    builder->pushStyle(defaultStyle);
    builder->addText(u"醉里挑灯看剑，梦回吹角连营。\n");

    skia::textlayout::TextStyle newStyle = defaultStyle;
    newStyle.setFontStyle(SkFontStyle::BoldItalic());
    newStyle.setLetterSpacing(8);
    newStyle.setColor(0xFFFFFF00);
    newStyle.setFontSize(26);
    newStyle.setDecoration(skia::textlayout::TextDecoration::kUnderline);
    builder->pushStyle(newStyle);
    builder->addText(u"八百里分麾下炙，五十弦翻塞外声。\n");
    builder->pop();
    builder->addText(u"沙场秋点兵。\n");

    auto paragraph = builder->Build();
    paragraph->layout(w);
    paragraph->paint(canvas, 10, 10);
}


void drawTextAndEmoji(SkCanvas* canvas)
{
    std::u16string text(uR"(轻舟已过万重山。😊)");
    sk_sp<skia::textlayout::FontCollection> fontCollection = sk_make_sp<skia::textlayout::FontCollection>();
    auto fontMgr = SkFontMgr_New_GDI();
    fontCollection->setDefaultFontManager(fontMgr);
    skia::textlayout::ParagraphStyle paraStyle;
    std::unique_ptr<skia::textlayout::ParagraphBuilder> builder = skia::textlayout::ParagraphBuilder::make(paraStyle, fontCollection);
    skia::textlayout::TextStyle defaultStyle;
    defaultStyle.setFontFamilies({ SkString{"Microsoft YaHei"},SkString{"Segoe UI Emoji"} });
    defaultStyle.setColor(0xff00ffff);
    defaultStyle.setFontSize(38);

    skia::textlayout::TextShadow shadow(0xFFFFFFFF, SkPoint::Make(2, 2), 2);
    defaultStyle.addShadow(shadow);

    builder->pushStyle(defaultStyle);
    builder->addText(text);
    std::unique_ptr<skia::textlayout::Paragraph> paragraph = builder->Build();
    paragraph->layout(w);
    paragraph->paint(canvas, 10, 10);
}

void paint(const HWND hWnd)
{
    if (w <= 0 || h <= 0)
        return;
    SkColor *surfaceMemory = new SkColor[w * h]{0xff000000};
    SkImageInfo info = SkImageInfo::MakeN32Premul(w, h);
    auto canvas = SkCanvas::MakeRasterDirect(info, surfaceMemory, 4 * w);

    drawMutiText(canvas.get());
    //drawMutiText2(canvas.get());
    //drawTextAndEmoji(canvas.get());

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