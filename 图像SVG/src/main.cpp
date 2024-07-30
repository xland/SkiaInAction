#include <windows.h>
#include <string>
#include "include/core/SkSurface.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkStream.h"

#include "modules/skshaper/utils/FactoryHelpers.h"
#include "modules/skresources/include/SkResources.h"
#include "src/utils/SkOSPath.h"
#include "modules/svg/include/SkSVGDOM.h"
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

void drawSVG(SkCanvas *canvas)
{
    std::wstring imgPath = L"D:\\project\\SkiaInAction\\图像SVG\\tiger.svg";
    auto pathStr = wideStrToStr(imgPath);
    SkFILEStream fs{ pathStr.data() };
    sk_sp<SkFontMgr> fontMgr = SkFontMgr_New_GDI();
    auto decodeType = skresources::ImageDecodeStrategy::kLazyDecode;
    auto fileResProvider = skresources::FileResourceProvider::Make(SkString{ pathStr }, decodeType);
    auto dataResProvider = skresources::DataURIResourceProviderProxy::Make(fileResProvider, decodeType,fontMgr);
    auto svgDom = SkSVGDOM::Builder()
        .setFontManager(fontMgr)
        .setResourceProvider(std::move(dataResProvider))
        .setTextShapingFactory(SkShapers::BestAvailable())
        .make(fs);
    svgDom->setContainerSize(SkSize::Make(w, h));
    svgDom->render(canvas);
}


void paint(const HWND hWnd)
{
    if (w <= 0 || h <= 0)
        return;
    SkColor *surfaceMemory = new SkColor[w * h]{0xff000000};
    SkImageInfo info = SkImageInfo::MakeN32Premul(w, h);
    auto canvas = SkCanvas::MakeRasterDirect(info, surfaceMemory, 4 * w);

    drawSVG(canvas.get());

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