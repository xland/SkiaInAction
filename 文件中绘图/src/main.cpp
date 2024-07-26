#include <windows.h>
#include "include/core/SkSurface.h"
#include "include/core/SkCanvas.h"
#include "include/encode/SkPngEncoder.h"
#include "include/core/SkStream.h"

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPTSTR lpCmdLine, _In_ int nCmdShow)
{
    SkImageInfo imgInfo = SkImageInfo::MakeN32Premul(800, 600);
    sk_sp<SkSurface> surface = SkSurfaces::Raster(imgInfo);
    SkCanvas* canvas = surface->getCanvas();
    SkPaint paint;
    paint.setColor(SK_ColorGREEN);
    SkRect rect;
    rect.setXYWH(10, 10, 100, 100);
    canvas->drawRect(rect, paint);
    SkPixmap pixmap;
    surface->peekPixels(&pixmap);

    SkFILEWStream stream("allen-image1.png");
    SkPngEncoder::Encode(&stream, pixmap, {});
    stream.flush();
    return 0;
}