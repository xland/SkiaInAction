#include <windows.h>
#include "include/core/SkSurface.h"
#include "include/core/SkCanvas.h"
#include "include/encode/SkPngEncoder.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkStream.h"
#include "include/codec/SkCodec.h"

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPTSTR lpCmdLine, _In_ int nCmdShow)
{
    //auto inputStream = std::make_unique<SkFILEStream>("allen-image1.png");
    //auto codec = SkCodec::MakeFromStream(std::move(inputStream));
    //auto imgInfo = codec->getInfo();
    //SkBitmap bitmap;
    //bitmap.allocPixels(imgInfo);
    //SkCodec::Result result = codec->getPixels(imgInfo, bitmap.getPixels(), bitmap.rowBytes());
    //auto surface = SkSurfaces::WrapPixels(bitmap.pixmap());


    auto imgInfo = SkImageInfo::MakeN32Premul(800, 600);
    auto surface = SkSurfaces::Raster(imgInfo);
    auto canvas = surface->getCanvas();
    SkPaint paint;
    //paint.setStroke(true);
    //paint.setStrokeWidth(3);
    paint.setColor(SK_ColorGREEN);//SK_ColorGREEN
    SkRect rect;
    rect.setXYWH(10, 10, 100, 100);
    //rect.setLTRB(10, 10, 110, 110);
    canvas->drawRect(rect,paint);
    auto image = surface->makeImageSnapshot();
    SkPixmap pixmap;
    image->peekPixels(&pixmap);
    SkFILEWStream stream("allen-image1.png");
    SkPngEncoder::Encode(&stream, pixmap, {});
    stream.flush();
    return 0;
}