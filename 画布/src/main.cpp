#include <windows.h>
#include <string>
#include "include/core/SkSurface.h"
#include "include/core/SkCanvas.h"
#include "include/effects/SkGradientShader.h"
#include "include/core/SkPath.h"
#include "include/core/SkPicture.h"
#include "include/core/SkPictureRecorder.h"
#include "src/base/SkRandom.h"
#include "include/core/SkStream.h"

#include <vector>

int w{400}, h{400};

void translateCanvas(SkCanvas *canvas)
{
    auto rect = SkRect::MakeXYWH(10, 10, 80, 80);
    SkPaint paint;
    paint.setColor(0xff00ffff);
    canvas->drawRect(rect, paint);
    canvas->translate(90, 90);
    paint.setColor(0xffff00ff);
    canvas->drawRect(rect, paint);
    canvas->translate(90, 90);
    paint.setColor(0xffffff00);
    canvas->drawRect(rect, paint);
}
void rotateCanvas(SkCanvas *canvas)
{
    auto rect = SkRect::MakeXYWH(w / 2 - 50, h / 2 - 100, 100, 200);
    SkPaint paint;
    paint.setColor(0xff00ffff);
    canvas->drawRect(rect, paint);
    canvas->rotate(-45, w / 2, h / 2);
    paint.setColor(0xffffff00);
    canvas->drawRect(rect, paint);
}
void skewCanvas(SkCanvas *canvas)
{
    auto rect = SkRect::MakeXYWH(60, 60, 80, 80);
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setColor(0xff00ffff);
    canvas->drawRect(rect, paint);

    canvas->save();
    canvas->translate(60, 140);
    canvas->skew(1, 0);
    paint.setColor(0xffff00ff);
    paint.setStroke(false);
    rect = SkRect::MakeXYWH(0, 0, 80, 80);
    canvas->drawRect(rect, paint);

    canvas->restore();
    canvas->translate(140, 60);
    canvas->skew(0, 1);
    paint.setColor(0xffffff00);
    paint.setStroke(false);
    rect = SkRect::MakeXYWH(0, 0, 80, 80);
    canvas->drawRect(rect, paint);
}
void saveCanvasLayer(SkCanvas *canvas)
{
    auto rect = SkRect::MakeXYWH(20, 20, 100, 100);
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setColor(0xff00ffff);
    canvas->drawRect(rect, paint);

    SkRect bounds = SkRect::MakeLTRB(0, 0, 160, 160);
    SkPaint layerPaint;
    layerPaint.setAlphaf(0.5f);
    canvas->saveLayer(&bounds, &layerPaint);

    rect = SkRect::MakeXYWH(80, 80, 200, 200);
    paint.setColor(0xffff00ff);
    canvas->drawRect(rect, paint);
    canvas->restore();
}

void drawEraser(SkCanvas* canvas)
{
    SkPaint paint;
    paint.setAntiAlias(true);
    SkPoint pts[2]{ SkPoint::Make(0, 0), SkPoint::Make(w, h) };
    SkColor colors[6]{ 0xFF00FFFF, 0xFFFF00FF, 0xFFFFFF00, 0xFF0000FF, 0xFF00FF00, 0xFFFF0000 };
    auto shader = SkGradientShader::MakeLinear(pts, colors, nullptr, 6, SkTileMode::kClamp);
    paint.setShader(shader);
    canvas->drawPaint(paint);

    canvas->saveLayer(SkRect::MakeXYWH(0, 0, w, h), nullptr);
    paint.setShader(nullptr);
    paint.setColor(0xFF00FFFF);
    auto r = std::min(w / 2 - 60, h / 2 - 60);
    canvas->drawCircle(w / 2, h / 2, r, paint);
    paint.setBlendMode(SkBlendMode::kClear);
    canvas->drawRect(SkRect::MakeXYWH(w / 2 - 50, h / 2 - 50, 100, 100), paint);
    canvas->restore();
}
void drawPixel(SkCanvas* canvas)
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
void clipCanvas(SkCanvas *canvas)
{
    canvas->save();
    auto rect = SkRect::MakeXYWH(0, 0, w / 2, h / 2);
    canvas->clipRect(rect);

    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setColor(0xff00ffff);
    auto r = std::min(w / 2, h / 2);
    canvas->drawCircle(SkPoint::Make(w / 2, h / 2), r, paint);
    canvas->restore();

    canvas->save();
    rect = SkRect::MakeXYWH(w / 2, h / 2, w / 2, h / 2);
    canvas->clipRect(rect);
    paint.setColor(0xffffff00);
    canvas->drawCircle(SkPoint::Make(w / 2, h / 2), r, paint);
    canvas->restore();
}
void recordCanvas(SkCanvas *canvas)
{
    SkPictureRecorder recorder;
    SkCanvas *canvasRecorder = recorder.beginRecording(w, h);
    auto maxR = std::min(w / 2, h / 2);
    SkRandom rnd;
    SkPaint paint;
    paint.setAntiAlias(true);
    for (size_t i = 0; i < 12; i++)
    {
        auto r = rnd.nextRangeU(10, maxR);
        auto x = rnd.nextRangeU(0 + r, w - r);
        auto y = rnd.nextRangeU(0 + r, h - r);
        auto color = rnd.nextRangeU(0xFF666666, 0xFFFFFFFF);
        paint.setColor(color);
        canvasRecorder->drawCircle(SkPoint::Make(x, y), r, paint);
    }
    sk_sp<SkPicture> picture = recorder.finishRecordingAsPicture();
    canvas->drawPicture(picture);
}

void serializePicture(SkCanvas* canvas) {
    SkPictureRecorder recorder;
    SkCanvas* canvasRecorder = recorder.beginRecording(w, h);
    auto maxR = std::min(w / 2, h / 2);
    SkRandom rnd;
    SkPaint paint;
    paint.setAntiAlias(true);
    for (size_t i = 0; i < 12; i++)
    {
        auto r = rnd.nextRangeU(10, maxR);
        auto x = rnd.nextRangeU(0 + r, w - r);
        auto y = rnd.nextRangeU(0 + r, h - r);
        auto color = rnd.nextRangeU(0xFF666666, 0xFFFFFFFF);
        paint.setColor(color);
        canvasRecorder->drawCircle(SkPoint::Make(x, y), r, paint);
    }
    sk_sp<SkPicture> picture = recorder.finishRecordingAsPicture();
    SkFILEWStream stream("picture-data.data");
    picture->serialize(&stream);
    stream.flush();
}

void deserializePicture(SkCanvas* canvas) {
    SkFILEStream stream("picture-data.data");
    auto picture = SkPicture::MakeFromStream(&stream);
    canvas->drawPicture(picture);
}

class AboutCB : public SkPicture::AbortCallback {
public:
    bool abort() override { 
        fCalls++;
        if (fCalls > 3) {
            return true;
        }
        return false;
    }
private:
    int fCalls = 0;
};

void playBackRecorder(SkCanvas* canvas) {    
    SkFILEStream stream("picture-data.data");
    auto picture = SkPicture::MakeFromStream(&stream);
    AboutCB aboutCB;
    picture->playback(canvas, &aboutCB);
}

void paint(const HWND hWnd)
{
    if (w <= 0 || h <= 0)
        return;
    SkColor *surfaceMemory = new SkColor[w * h]{0xff000000};
    SkImageInfo info = SkImageInfo::MakeN32Premul(w, h);
    auto canvas = SkCanvas::MakeRasterDirect(info, surfaceMemory, 4 * w);
    // translateCanvas(canvas.get());
    // rotateCanvas(canvas.get());
    // skewCanvas(canvas.get());
    // saveCanvasLayer(canvas.get());
    // drawEraser(canvas.get());
    // drawPixel(canvas.get());
    // clipCanvas(canvas.get());
    // recordCanvas(canvas.get());
    // serializePicture(canvas.get());
    // deserializePicture(canvas.get());
    playBackRecorder(canvas.get());

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