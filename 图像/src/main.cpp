#include <windows.h>
#include <string>
#include "include/core/SkSurface.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkRRect.h"
#include "include/core/SkPath.h"

#include "include/core/SkStream.h"
#include "include/core/SkBitmap.h"
#include "include/codec/SkCodec.h"

#include "include/core/SkMaskFilter.h"
#include "include/core/SkBlurTypes.h"
#include "include/core/SkColorFilter.h"
#include "include/effects/SkImageFilters.h"

#include "include/encode/SkPngEncoder.h"
#include "include/encode/SkJpegEncoder.h"
#include "include/encode/SkWebpEncoder.h"


#include "include/codec/SkPngDecoder.h"
#include "include/codec/SkJpegDecoder.h"
#include "include/codec/SkWebpDecoder.h"

int w{400}, h{400};

std::string wideStrToStr(const std::wstring& wstr)
{
    const int count = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.length(), NULL, 0, NULL, NULL);
    std::string str(count, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &str[0], count, NULL, NULL);
    return str;
}

sk_sp<SkImage> getImg() {
    std::wstring imgPath = L"D:\\project\\SkiaInAction\\图像处理\\original.png";
    auto pathStr = wideStrToStr(imgPath);
    std::unique_ptr<SkFILEStream> stream = SkFILEStream::Make(pathStr.data());
    SkCodec::Result result;
    std::unique_ptr<SkCodec> codec = SkCodec::MakeFromStream(std::move(stream), &result);
    if (result != SkCodec::kSuccess) {
        return nullptr;
    }
    SkImageInfo imgInfo = codec->getInfo();
    SkBitmap bitmap;
    bitmap.allocPixels(imgInfo);
    result = codec->getPixels(imgInfo, bitmap.getPixels(), bitmap.rowBytes());
    if (SkCodec::kSuccess != result) {
        return nullptr;
    }
    bitmap.setImmutable();
    return bitmap.asImage();
}

sk_sp<SkImage> getImg2() {
    std::wstring imgPath = L"D:\\project\\SkiaInAction\\图像处理\\original.png";
    auto pathStr = wideStrToStr(imgPath);
    sk_sp<SkData> data{ SkData::MakeFromFileName(pathStr.data()) };
    std::unique_ptr<SkCodec> codec = SkCodec::MakeFromData(data);
    SkImageInfo imgInfo = codec->getInfo();
    SkBitmap bitmap;
    bitmap.allocPixels(imgInfo);
    codec->getPixels(imgInfo, bitmap.getPixels(), bitmap.rowBytes());
    bitmap.setImmutable();
    return bitmap.asImage();
}

sk_sp<SkImage> getImg3() {
    std::wstring imgPath = L"D:\\project\\SkiaInAction\\图像处理\\original.png";
    auto pathStr = wideStrToStr(imgPath);
    sk_sp<SkData> data{ SkData::MakeFromFileName(pathStr.data()) };
    auto img = SkImages::DeferredFromEncodedData(data);
    return img;
}

sk_sp<SkImage> getImg4() {
    //std::wstring imgPath = L"D:\\project\\SkiaInAction\\图像处理\\original.png";
    //auto pathStr = wideStrToStr(imgPath);
    //sk_sp<SkData> data{ SkData::MakeFromFileName(pathStr.data()) };
    //std::vector<SkCodecs::Decoder> decoders;
    //decoders.push_back(SkPngDecoder::Decoder());
    //decoders.push_back(SkJpegDecoder::Decoder());
    //decoders.push_back(SkWebpDecoder::Decoder());
    //std::unique_ptr<SkCodec> codec = SkCodec::MakeFromData(data, decoders);

    std::wstring imgPath = L"D:\\project\\SkiaInAction\\图像处理\\original.png";
    auto pathStr = wideStrToStr(imgPath);
    std::unique_ptr<SkFILEStream> stream = SkFILEStream::Make(pathStr.data());
    SkCodec::Result result;
    std::vector<SkCodecs::Decoder> decoders;
    decoders.push_back(SkPngDecoder::Decoder());
    decoders.push_back(SkJpegDecoder::Decoder());
    decoders.push_back(SkWebpDecoder::Decoder());
    std::unique_ptr<SkCodec> codec = SkCodec::MakeFromStream(std::move(stream), decoders,&result);

    SkImageInfo imgInfo = codec->getInfo();
    SkBitmap bitmap;
    bitmap.allocPixels(imgInfo);
    codec->getPixels(imgInfo, bitmap.getPixels(), bitmap.rowBytes());
    bitmap.setImmutable();
    return bitmap.asImage();
}

void encodeImg() {
    std::wstring srcPath = L"D:\\project\\SkiaInAction\\图像处理\\original.png";
    auto srcStr = wideStrToStr(srcPath);
    sk_sp<SkData> data{ SkData::MakeFromFileName(srcStr.data()) };
    auto codec = SkCodec::MakeFromData(data);
    auto imgInfo = codec->getInfo();
    SkColor* byteMem = new SkColor[imgInfo.width() * imgInfo.height()]();
    SkPixmap pixmap(imgInfo, byteMem,imgInfo.minRowBytes());
    codec->getPixels(pixmap);
    
    std::wstring targetPath = L"D:\\project\\SkiaInAction\\图像处理\\original.jpg";
    auto targetStr = wideStrToStr(targetPath);
    SkFILEWStream stream(targetStr.data());

    SkJpegEncoder::Options options;
    options.fQuality = 80; //90kb -> 36kb
    SkJpegEncoder::Encode(&stream, pixmap, options);

    //SkWebpEncoder::Options options;
    //options.fQuality = 80;
    //SkWebpEncoder::Encode(&stream, pixmap, options);

    stream.flush();
    delete[] byteMem;
}

void drawImage(SkCanvas *canvas)
{
    canvas->clear(0xFFFFFFFF);
    //auto img = getImg();
    //auto img = getImg2();
    auto img = getImg3();
    //auto img = getImg4();
    canvas->drawImage(img, 0, 0);    
}

void drawImgRect(SkCanvas* canvas)
{
    canvas->clear(0xFFFFFFFF);
    auto img = getImg();
    auto rect = SkRect::MakeXYWH(0, 0, w, h);
    SkSamplingOptions imgOption{};
    //SkSamplingOptions imgOption{ SkFilterMode::kLinear, SkMipmapMode::kLinear };
    canvas->drawImageRect(img, rect, imgOption);
    //SkPaint paint;
    //paint.setAntiAlias(true);
    //canvas->drawImageRect(img, rect, imgOption,&paint);
}

void blurImg(SkCanvas* canvas) {
    canvas->clear(0xFFFFFFFF);
    auto img = getImg();
    auto rect = SkRect::MakeXYWH(0, 0, w, h);
    SkSamplingOptions imgOption{ SkFilterMode::kLinear, SkMipmapMode::kLinear };
    SkPaint paint;
    sk_sp<SkImageFilter> filter = SkImageFilters::Blur(8, 8, nullptr, {});
    paint.setImageFilter(filter);
    canvas->drawImageRect(img, rect, imgOption, &paint);
}

void imgBlendColor(SkCanvas* canvas) {
    canvas->clear(0xFFFFFFFF);
    auto img = getImg();
    auto rect = SkRect::MakeXYWH(0, 0, w, h);
    SkSamplingOptions imgOption{ SkFilterMode::kLinear, SkMipmapMode::kLinear };
    SkPaint paint;
    sk_sp<SkColorFilter> filter = SkColorFilters::Blend(0xff000000, SkBlendMode::kSrcATop);  //SkBlendMode::kXor
    paint.setColorFilter(filter);
    canvas->drawImageRect(img, rect, imgOption, &paint);
}

void imgColorFilter(SkCanvas* canvas) {
    canvas->clear(0xFFFFFFFF);
    auto img = getImg();
    auto rect = SkRect::MakeXYWH(0, 0, w, h);
    SkSamplingOptions imgOption{ SkFilterMode::kLinear, SkMipmapMode::kLinear };
    SkPaint paint;
    SkScalar colorMatrix[20] = {
    0, 0, 1, 0, 0,
    0, 1, 0, 0, 0,
    1, 0, 0, 0, 0,
    0, 0, 0, 1, 0 }; // mix G and A.
    sk_sp<SkColorFilter> filter = SkColorFilters::Matrix(colorMatrix);
    paint.setColorFilter(filter);
    canvas->drawImageRect(img, rect, imgOption, &paint);
}

void imgHSLA(SkCanvas* canvas) {
    canvas->clear(0xFFFFFFFF);
    auto img = getImg();
    auto rect = SkRect::MakeXYWH(0, 0, w, h);
    SkSamplingOptions imgOption{ SkFilterMode::kLinear, SkMipmapMode::kLinear };
    SkPaint paint;
    //SkColorFilter* colorFilter = SkColorFilters::HSLAMatrix(hueShift, saturationScale, 1, 0);
    //paint.setColorFilter(filter);
    canvas->drawImageRect(img, rect, imgOption, &paint);
}


void paint(const HWND hWnd)
{
    if (w <= 0 || h <= 0)
        return;
    SkColor *surfaceMemory = new SkColor[w * h]{0xff000000};
    SkImageInfo info = SkImageInfo::MakeN32Premul(w, h);
    auto canvas = SkCanvas::MakeRasterDirect(info, surfaceMemory, 4 * w);
    drawImage(canvas.get());
    //encodeImg();
    //drawImgRect(canvas.get());
    //blurImg(canvas.get());
    //imgColorFilter(canvas.get());
    //imgBlendColor(canvas.get());

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