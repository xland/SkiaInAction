#include <windows.h>
#include <string>
#include "include/core/SkSurface.h"
#include "include/core/SkCanvas.h"
#include "src/base/SkAutoMalloc.h"
#include "include/core/SkPath.h"
#include "include/pathops/SkPathOps.h"
#include "include/utils/SkParsePath.h"
#include "include/core/SkColorFilter.h"
#include "include/effects/SkDiscretePathEffect.h"
#include "include/effects/SkDashPathEffect.h"
#include "include/utils/SkShadowUtils.h"
#include "include/core/SkPoint3.h"


#include "include/core/SkMaskFilter.h"
#include "include/core/SkBlurTypes.h"

int w{400}, h{400};
SkAutoMalloc surfaceMemory;

void drawMultiPath(SkCanvas* canvas) {
    SkPath path0;
    path0.addRect(SkRect::MakeXYWH(100, 100, 100, 100));
    SkPath path1;
    path1.addRect(SkRect::MakeXYWH(150, 150, 100, 100));
    SkPath path;
    Op(path0, path1, SkPathOp::kXOR_SkPathOp, &path);
    SkPaint paint;
    paint.setColor(0xFF00FFFF);
    canvas->drawPath(path,paint); 
}

void drawBlendMode(SkCanvas* canvas) {
    canvas->clear(0);
    SkPaint paint;
    paint.setColor(0xFF00FFFF);
    auto rect1 = SkRect::MakeLTRB(60, h / 2 - 30, w - 60, h / 2 + 30);
    canvas->drawRect(rect1, paint);
    paint.setColor(0xFFFFFF00);
    paint.setBlendMode(SkBlendMode::kSrcOut);
    auto rect2 = SkRect::MakeLTRB(w / 2 - 30, 60, w / 2 + 30, h - 60);
    canvas->drawRect(rect2, paint);
}

void drawEraser(SkCanvas* canvas) {
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setColor(0xFF00FFFF);
    auto r = std::min(w/2-60, h/2-60);
    canvas->drawCircle(w/2,h/2,r,paint);
    paint.setBlendMode(SkBlendMode::kClear);
    canvas->drawRect(SkRect::MakeXYWH(w / 2 - 50, h / 2 - 50,100,100), paint);
}

void drawBlur(SkCanvas* canvas) {
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setColor(0xFF00FFFF);
    auto filter = SkMaskFilter::MakeBlur(SkBlurStyle::kNormal_SkBlurStyle, 8);
    SkMaskFilter::MakeBlur(gStyles[i], 8)
    paint.setMaskFilter(filter);
    auto r = std::min(w / 2 - 60, h / 2 - 60);
    canvas->drawCircle(w / 2, h / 2, r, paint);
}

void drawPathEffect(SkCanvas* canvas) {
    SkPaint paint;
    paint.setColor(0xFF00FFFF);
    paint.setStroke(true);
    paint.setStrokeWidth(8);
    paint.setStrokeCap(SkPaint::Cap::kRound_Cap);
    paint.setStrokeJoin(SkPaint::kRound_Join);
    paint.setAntiAlias(true);
    //auto effect = SkDiscretePathEffect::Make(6, 8);
    SkScalar kIntervals[] = { 12, 16,28,18 };
    auto effect = SkDashPathEffect::Make(kIntervals, 4, 25);
    paint.setPathEffect(effect);
    auto rect = SkRect::MakeLTRB(60, 60, w - 60, h - 60);
    canvas->drawRect(rect, paint);
}

void drawPathShadow(SkCanvas* canvas) {
    SkPath path;
    path.addRect(SkRect::MakeLTRB(80, 80, w - 80, h - 80));
    auto zPlaneParams = SkPoint3::Make(0, 0, 80);// 定义阴影与 z 平面的关系    
    auto lightPos = SkPoint3::Make(0, 0, 0);// 定义光源的位置和半径
    SkShadowUtils::DrawShadow(canvas, path, zPlaneParams, lightPos, 80.f, 0xFF00FFFF, SK_ColorTRANSPARENT);
}

void setPixel()
{
    surfaceMemory.reset(h * 4 * w);
    SkImageInfo info = SkImageInfo::MakeN32Premul(w, h);
    auto canvas = SkCanvas::MakeRasterDirect(info, surfaceMemory.get(), 4 * w);
    canvas->clear(SK_ColorBLACK);
    //drawMultiPath(canvas.get());
    //drawBlendMode(canvas.get());
    //drawEraser(canvas.get());
    //drawPathEffect(canvas.get());
    //drawPathShadow(canvas.get());
    drawBlur(canvas.get());
}

void paint(const HWND hWnd)
{
    PAINTSTRUCT ps;
    auto dc = BeginPaint(hWnd, &ps);
    BITMAPINFO info = {sizeof(BITMAPINFOHEADER), w, 0 - h, 1, 32, BI_RGB, h * 4 * w, 0, 0, 0, 0};
    StretchDIBits(dc, 0, 0, w, h, 0, 0, w, h, surfaceMemory.get(), &info, DIB_RGB_COLORS, SRCCOPY);
    ReleaseDC(hWnd, dc);
    EndPaint(hWnd, &ps);
    surfaceMemory.reset(0);
}

LRESULT CALLBACK wndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_SIZE:
    {
        w = LOWORD(lParam);
        h = HIWORD(lParam);
        if (wParam != SIZE_MINIMIZED)
        {
            setPixel();
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