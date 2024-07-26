#include <windows.h>
#include <string>
#include "include/core/SkSurface.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkPath.h"
#include "include/pathops/SkPathOps.h"
#include "include/utils/SkParsePath.h"
#include "include/core/SkColorFilter.h"
#include "include/effects/SkDiscretePathEffect.h"
#include "include/effects/SkDashPathEffect.h"
#include "include/utils/SkShadowUtils.h"
#include "include/core/SkPoint3.h"
#include "include/effects/SkGradientShader.h"
#include "include/core/SkMaskFilter.h"
#include "include/core/SkBlurTypes.h"
#include "include/effects/SkImageFilters.h"
#include "include/core/SkRRect.h"
#include <vector>

int w{400}, h{400};

void drawMultiPath(SkCanvas *canvas)
{
    SkPath path0;
    path0.addRect(SkRect::MakeXYWH(100, 100, 100, 100));
    SkPath path1;
    path1.addRect(SkRect::MakeXYWH(150, 150, 100, 100));
    SkPath path;
    Op(path0, path1, SkPathOp::kXOR_SkPathOp, &path);
    SkPaint paint;
    paint.setColor(0xFF00FFFF);
    canvas->drawPath(path, paint);
}

void drawBlur(SkCanvas *canvas)
{
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setColor(0xFF00FFFF);
    auto filter = SkMaskFilter::MakeBlur(SkBlurStyle::kNormal_SkBlurStyle, 8);
    paint.setMaskFilter(filter);
    auto r = std::min(w / 2 - 60, h / 2 - 60);
    canvas->drawCircle(w / 2, h / 2, r, paint);
}

void drawPathEffect(SkCanvas *canvas)
{
    SkPaint paint;
    paint.setColor(0xFF00FFFF);
    paint.setStroke(true);
    paint.setStrokeWidth(8);
    paint.setStrokeCap(SkPaint::Cap::kRound_Cap);
    paint.setStrokeJoin(SkPaint::kRound_Join);
    paint.setAntiAlias(true);
    // auto effect = SkDiscretePathEffect::Make(6, 8);
    SkScalar kIntervals[] = {12, 16, 28, 18};
    auto effect = SkDashPathEffect::Make(kIntervals, 4, 25);
    paint.setPathEffect(effect);
    auto rect = SkRect::MakeLTRB(60, 60, w - 60, h - 60);
    canvas->drawRect(rect, paint);
}

void drawPathShadow(SkCanvas *canvas)
{
    SkPath path;
    path.addRect(SkRect::MakeLTRB(80, 80, w - 80, h - 80));
    auto zPlaneParams = SkPoint3::Make(0, 0, 80);
    auto lightPos = SkPoint3::Make(0, 0, 0);
    SkShadowUtils::DrawShadow(canvas, path, zPlaneParams, lightPos, 80.f, 0xFF00FFFF, SK_ColorTRANSPARENT);
}

void drawPathShadow2(SkCanvas* canvas) {
    SkPaint paint;
    paint.setStroke(false);
    paint.setColor(0xFF00FFFF);
    paint.setAntiAlias(true);
    sk_sp<SkImageFilter> dropShadowFilter = SkImageFilters::DropShadow(
        0, 0, // 阴影偏移 (dx, dy)
        18.0f, 18.0f, // 模糊半径 (sigmaX, sigmaY)
        0xffffffff, // 阴影颜色
        nullptr // 输入图像过滤器
    );
    paint.setImageFilter(dropShadowFilter); 
    auto rect = SkRect::MakeXYWH(w / 2 - 100, h / 2 - 100, 200, 200);
    SkVector radii[4]{
        {16, 16},  // 矩形左上角圆角尺寸；
        {16, 16},  // 矩形右上角圆角尺寸；
        {16, 16},  // 矩形右下角圆角尺寸；
        {16, 16}   // 矩形左下角圆角尺寸；
    };
    SkRRect rr;
    rr.setRectRadii(rect, radii);
    canvas->drawRRect(rr, paint);
}

void drawPixel(SkCanvas *canvas)
{
    SkImageInfo info = SkImageInfo::MakeN32Premul(w, h);
    std::vector<SkColor> pixels;
    for (int y = 0; y < h; ++y)
    {
        for (int x = 0; x < w; ++x)
        {
            auto val = x % 30;
            if (val == 0)
            {
                pixels.push_back(0xFFFF00FF);
            }
            else if (val == 10)
            {
                pixels.push_back(0xFFFFFF00);
            }
            else if (val == 20)
            {
                pixels.push_back(0xFF00FFFF);
            }
            else
            {
                pixels.push_back(0xFF0000FF);
            }
        }
    }
    canvas->writePixels(info, &pixels.front(), w * 4, 0, 0);
}

void paint(const HWND hWnd)
{
    if (w <= 0 || h <= 0) return;
    SkColor* surfaceMemory = new SkColor[w * h]{ SK_ColorBLACK };
    SkImageInfo info = SkImageInfo::MakeN32Premul(w, h);
    auto canvas = SkCanvas::MakeRasterDirect(info, surfaceMemory, 4 * w);
    // drawMultiPath(canvas.get());
    // drawPathEffect(canvas.get());
    // drawPathShadow(canvas.get());
    drawPathShadow2(canvas.get());
    // drawBlur(canvas.get());
    // drawPixel(canvas.get());

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