#include <windows.h>
#include <string>
#include <format>
#include "include/core/SkSurface.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkRRect.h"
#include "include/core/SkPath.h"

#include "include/core/SkFontMgr.h"
#include "include/core/SkFontStyle.h"
#include "include/ports/SkTypeface_win.h"
#include "include/core/SkFont.h"
#include "include/effects/SkGradientShader.h"

#include <thread>
#include <mutex>



int w{500}, h{500};
HWND hwnd;
SkColor* surfaceMemory{nullptr};
std::mutex locker;
std::atomic<bool> windowClosed(false);
std::atomic<bool> threadClosed(false);

float degrees{ 1.f };
float startY;
float percent{0.f};
float circleR{ 200 };
float rectR{ 280 };
float rectW;

SkCanvas* getCanvas() {
    static std::unique_ptr<SkCanvas> canvas;
    if (!canvas.get()) {
        SkImageInfo info = SkImageInfo::MakeN32Premul(w, h);
        canvas = SkCanvas::MakeRasterDirect(info, surfaceMemory, 4 * w);
        return canvas.get();
    }
    auto info = canvas->imageInfo();
    if (w != info.width() || h != info.height()) {
        SkImageInfo newInfo = SkImageInfo::MakeN32Premul(w, h);
        canvas = SkCanvas::MakeRasterDirect(newInfo, surfaceMemory, 4 * w);
    }
    return canvas.get();
}

SkFont* getFont() {
    static std::shared_ptr<SkFont> font;
    if (!font) {
        auto fontMgr = SkFontMgr_New_GDI();
        SkFontStyle fontStyle = SkFontStyle::Normal();
        auto typeFace = fontMgr->matchFamilyStyle("Microsoft YaHei", fontStyle);
        font = std::make_shared<SkFont>(typeFace, 36);
    }
    return font.get();
}


void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setAntiAlias(true);
    auto x = w / 2;
    auto y = h / 2;
    
    //绘制一个线性渐变色圆形
    SkPoint pts[2]{ SkPoint::Make(0, y-circleR), SkPoint::Make(0, y+circleR) };
    SkColor colors[2]{ 0xFFbae0ff, 0xFF0958d9 };
    sk_sp<SkShader> shader = SkGradientShader::MakeLinear(pts, colors, nullptr, 2, SkTileMode::kClamp);
    paint.setShader(shader);
    canvas->drawCircle(x, y, circleR, paint);
    paint.setShader(nullptr);
    //保存画布
    canvas->saveLayer(SkRect::MakeXYWH(0, 0, w, h), nullptr);    
    //绘制一个白色背景的圆形，盖住线性渐色变圆形
    paint.setColor(0xFFFFFFFF);
    canvas->drawCircle(x, y, circleR, paint);
    //设置擦除模式
    paint.setBlendMode(SkBlendMode::kClear);
    SkRect rect;
    rect.setXYWH(x - rectW / 2, startY - (circleR * 2 * (percent / 100)), rectW, rectW);
    canvas->rotate(degrees, rect.centerX(), rect.centerY());
    SkVector radii[4]{ {rectR, rectR},{rectR, rectR},{rectR, rectR},{rectR, rectR} };
    SkRRect rr;
    rr.setRectRadii(rect, radii);
    paint.setColor(0xFFFFFFFF);
    //绘制圆角矩形，用于擦除白色背景的圆形，露出线性渐变色的圆形
    canvas->drawRRect(rr, paint);
    canvas->restore();

    paint.setBlendMode(SkBlendMode::kSrc);
    paint.setColor(0xff73d13d);
    auto text = std::format("{}%", (int)std::round(percent));
    auto font = getFont();
    SkRect measureRect;
    //在圆形中间绘制完成百分比
    font->measureText(text.c_str(), text.size(), SkTextEncoding::kUTF8, &measureRect);

    canvas->drawString(text.data(),
        x - measureRect.width() / 2 - measureRect.fLeft,
        y - measureRect.height() / 2 - measureRect.fTop,
        *font, paint);

    degrees += 1.8;  //圆角矩形旋转角度
    if (degrees > 360) {
        degrees = 0; 
    }
    percent += 0.1;  //圆角矩形向上移动的百分比
    if (percent > 100) {
        percent = 0;
        std::this_thread::sleep_for(std::chrono::milliseconds(6000));
    }    
}
void animateStart()
{
    auto t = std::thread([&]() {

        while (true)
        {
            std::unique_lock guard(locker);
            auto canvas = getCanvas();
            canvas->clear(0xff000000);
            draw(canvas);
            InvalidateRect(hwnd, nullptr, false);
            guard.unlock();
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
            if (windowClosed.load()) {
                threadClosed.store(true);
                break;
            }
        }
        });
    t.detach();
}

void paint(const HWND hWnd)
{
    if (w <= 0 || h <= 0)
        return;  
    PAINTSTRUCT ps;
    auto dc = BeginPaint(hWnd, &ps);
    BITMAPINFO bmpInfo = { sizeof(BITMAPINFOHEADER), w, 0 - h, 1, 32, BI_RGB, h * 4 * w, 0, 0, 0, 0 };
    StretchDIBits(dc, 0, 0, w, h, 0, 0, w, h, surfaceMemory, &bmpInfo, DIB_RGB_COLORS, SRCCOPY);
    ReleaseDC(hWnd, dc);
    EndPaint(hWnd, &ps);
}

LRESULT CALLBACK wndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_SIZE:
    {
        w = LOWORD(lParam);
        h = HIWORD(lParam);
        if (surfaceMemory) {
            std::unique_lock guard(locker);
            delete[] surfaceMemory;
            surfaceMemory = new SkColor[w * h]{ 0xff000000 };
            guard.unlock();
        }
        break;
    }
    case WM_PAINT:
    {
        paint(hWnd);
        break;
    }
    case WM_CLOSE:
    {
        windowClosed.store(true);
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
    hwnd = CreateWindow(clsName.c_str(), clsName.c_str(), WS_OVERLAPPEDWINDOW,
                             CW_USEDEFAULT, CW_USEDEFAULT, w, h,
                             nullptr, nullptr, hinstance, nullptr);
    ShowWindow(hwnd, SW_SHOW);
    rectW = circleR * 2+360;
    startY = h / 2 + circleR;
    surfaceMemory = new SkColor[w * h]{ 0xff000000 };
    animateStart();
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
    while (!threadClosed.load()) {
        std::this_thread::yield();
    }
    return 0;
}