#include <windows.h>
#include <string>
#include "include/core/SkImage.h"

#include "include/core/SkSurface.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkRRect.h"
#include "include/core/SkPath.h"

#include "include/core/SkStream.h"
#include "include/core/SkBitmap.h"
#include "include/codec/SkCodec.h"

#include "include/codec/SkPngDecoder.h"
#include "include/codec/SkJpegDecoder.h"
#include "include/codec/SkWebpDecoder.h"

#include "src/base/SkBase64.h"

int w{400}, h{400};

std::string wideStrToStr(const std::wstring& wstr)
{
    const int count = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.length(), NULL, 0, NULL, NULL);
    std::string str(count, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &str[0], count, NULL, NULL);
    return str;
}

sk_sp<SkImage> getImgFromStream() {
    std::wstring imgPath = L"D:\\project\\SkiaInAction\\图像解码\\original.png";
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

sk_sp<SkImage> getImgFromData() {
    std::wstring imgPath = L"D:\\project\\SkiaInAction\\图像解码\\original.png";
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

sk_sp<SkImage> getImgDefferd() {
    std::wstring imgPath = L"D:\\project\\SkiaInAction\\图像解码\\original.png";
    auto pathStr = wideStrToStr(imgPath);
    sk_sp<SkData> data{ SkData::MakeFromFileName(pathStr.data()) };
    auto img = SkImages::DeferredFromEncodedData(data);
    return img;
}

sk_sp<SkImage> getImgBase64() {
    std::string dataStr{ "data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAQAAAAEACAIAAADTED8xAAADMElEQVR4nOzVwQnAIBQFQYXff81RUkQCOyDj1YOPnbXWPmeTRef+/3O/OyBjzh3CD95BfqICMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMK0CMO0TAAD//2Anhf4QtqobAAAAAElFTkSuQmCC" };
    dataStr = dataStr.substr(22);
    size_t dataLen;
    if (SkBase64::Decode(dataStr.c_str(), dataStr.size(), nullptr, &dataLen) != SkBase64::kNoError) {
        return nullptr;
    }
    sk_sp<SkData> decodedData = SkData::MakeUninitialized(dataLen);
    void* rawData = decodedData->writable_data();
    if (SkBase64::Decode(dataStr.c_str(), dataStr.size(), rawData, &dataLen) != SkBase64::kNoError) {
        return nullptr;
    }
    auto image = SkImages::DeferredFromEncodedData(std::move(decodedData));
    return image;
}

void paint(const HWND hWnd)
{
    if (w <= 0 || h <= 0)
        return;
    SkColor *surfaceMemory = new SkColor[w * h]{0xff000000};
    SkImageInfo info = SkImageInfo::MakeN32Premul(w, h);
    auto canvas = SkCanvas::MakeRasterDirect(info, surfaceMemory, 4 * w);

    canvas->clear(0xFFFFFFFF);
    auto img = getImgFromStream();
    //auto img = getImgFromData();
    //auto img = getImgDefferd();
    //auto img = getImgDeprecated();
    //auto img = getImgBase64();
    canvas->drawImage(img, 0, 0);

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