- skunicode_core.lib skunicode_icu.lib skparagraph.lib skshaper.lib 
> We haven't yet created a way to encode the ICU data for assembly on Windows,so we use a helper library to load icudtl.dat from the harddrive.


```c++
auto fFontCollection = sk_make_sp<skia::textlayout::FontCollection>();
fFontCollection->setDefaultFontManager(SkFontMgr_New_GDI());

skia::textlayout::ParagraphStyle paragraph_style;
paragraph_style.turnHintingOff();

skia::textlayout::TextStyle textStyle;
textStyle.setFontFamilies({ SkString("Microsoft YaHei") });
textStyle.setColor(SK_ColorBLACK);
//textStyle.setBackgroundPaint(*paint);
textStyle.setFontSize(24);
auto builder = skia::textlayout::ParagraphBuilder::make(paragraph_style, fFontCollection);
builder->pushStyle(textStyle);

std::wstring wstr = L"你好，世界！";
std::u16string chinese_text(wstr.begin(), wstr.end());
builder->addText(chinese_text);
auto fParagraph = builder->Build();
fParagraph->layout(360);
fParagraph->paint(win->canvas.get(), 160, 160);
```