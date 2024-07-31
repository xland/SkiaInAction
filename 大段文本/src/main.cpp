#include <windows.h>
#include <string>
#include "include/core/SkSurface.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkStream.h"

#include "modules/skshaper/include/SkShaper.h"
#include "modules/skparagraph/include/Paragraph.h"
#include "modules/skparagraph/include/TypefaceFontProvider.h"
#include "modules/skparagraph/src/ParagraphBuilderImpl.h"
#include "modules/skparagraph/src/ParagraphImpl.h"
#include "modules/skparagraph/src/TextLine.h"
#include "modules/skparagraph/utils/TestFontCollection.h"
#include "modules/skshaper/utils/FactoryHelpers.h"

#include "include/core/SkFontMgr.h"
#include "include/ports/SkTypeface_win.h"

int w{800}, h{800};

std::string wideStrToStr(const std::wstring& wstr)
{
    const int count = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.length(), NULL, 0, NULL, NULL);
    std::string str(count, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &str[0], count, NULL, NULL);
    return str;
}

void drawMutiText(SkCanvas *canvas)
{
    std::u16string text(uR"(春江潮水连海平，海上明月共潮生。滟滟随波千万里，何处春江无月明！
江流宛转绕芳甸，月照花林皆似霰。空里流霜不觉飞，汀上白沙看不见。
江天一色无纤尘，皎皎空中孤月轮。江畔何人初见月？江月何年初照人？
人生代代无穷已，江月年年望相似。不知江月待何人，但见长江送流水。
白云一片去悠悠，青枫浦上不胜愁。谁家今夜扁舟子？何处相思明月楼？
可怜楼上月裴回，应照离人妆镜台。玉户帘中卷不去，捣衣砧上拂还来。
此时相望不相闻，愿逐月华流照君。鸿雁长飞光不度，鱼龙潜跃水成文。
昨夜闲潭梦落花，可怜春半不还家。江水流春去欲尽，江潭落月复西斜。
斜月沉沉藏海雾，碣石潇湘无限路。不知乘月几人归，落月摇情满江树)");

    sk_sp<skia::textlayout::FontCollection> fontCollection = sk_make_sp<skia::textlayout::FontCollection>();
    auto fontMgr = SkFontMgr_New_GDI();
    fontCollection->setDefaultFontManager(fontMgr);
    fontCollection->enableFontFallback();
    skia::textlayout::ParagraphStyle paraStyle;
    auto builder = skia::textlayout::ParagraphBuilder::make(paraStyle, fontCollection);
    skia::textlayout::TextStyle defaultStyle;
    defaultStyle.setFontFamilies({ SkString{"Microsoft YaHei"} });
    defaultStyle.setColor(0xff00ffff);
    defaultStyle.setFontSize(18);
    builder->pushStyle(defaultStyle);
    builder->addText(text);
    builder->pop();
    auto paragraph = builder->Build();
    paragraph->layout(w);
    paragraph->paint(canvas, 0, 0);

    return;




    /*std::u16string text(uR"(春江潮水连海平，海上明月共潮生。滟滟随波千万里，何处春江无月明！
江流宛转绕芳甸，月照花林皆似霰。空里流霜不觉飞，汀上白沙看不见。
江天一色无纤尘，皎皎空中孤月轮。江畔何人初见月？江月何年初照人？
人生代代无穷已，江月年年望相似。不知江月待何人，但见长江送流水。
白云一片去悠悠，青枫浦上不胜愁。谁家今夜扁舟子？何处相思明月楼？
可怜楼上月裴回，应照离人妆镜台。玉户帘中卷不去，捣衣砧上拂还来。
此时相望不相闻，愿逐月华流照君。鸿雁长飞光不度，鱼龙潜跃水成文。
昨夜闲潭梦落花，可怜春半不还家。江水流春去欲尽，江潭落月复西斜。
斜月沉沉藏海雾，碣石潇湘无限路。不知乘月几人归，落月摇情满江树)");
    auto fontCollection = sk_make_sp<skia::textlayout::FontCollection>();
    auto fontMgr = SkFontMgr_New_GDI();
    fontCollection->setDefaultFontManager(fontMgr);
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setColor(SK_ColorBLACK);
    skia::textlayout::TextStyle textStyle;
    textStyle.setForegroundColor(paint);
    textStyle.setFontFamilies({ SkString("Microsoft YaHei") });
    textStyle.setFontSize(42.0f);
    textStyle.setLetterSpacing(-0.05f);
    textStyle.setHeightOverride(true);
    skia::textlayout::ParagraphStyle paragraphStyle;
    paragraphStyle.setTextStyle(textStyle);
    paragraphStyle.setTextAlign(skia::textlayout::TextAlign::kLeft);
    auto factory = SkShapers::BestAvailable();
    sk_sp<SkUnicode> unicodeObj(factory->getUnicode());
    skia::textlayout::ParagraphBuilderImpl builder(paragraphStyle, fontCollection, unicodeObj);
    const char* hello = "test test test";
    builder.addText(hello,strlen(hello));
    auto paragraph = builder.Build();
    paragraph->layout(w);
    canvas->translate(10, 10);
    paragraph->paint(canvas, 0, 0);*/
}


void paint(const HWND hWnd)
{
    if (w <= 0 || h <= 0)
        return;
    SkColor *surfaceMemory = new SkColor[w * h]{0xff000000};
    SkImageInfo info = SkImageInfo::MakeN32Premul(w, h);
    auto canvas = SkCanvas::MakeRasterDirect(info, surfaceMemory, 4 * w);

    drawMutiText(canvas.get());

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