```c++
#include "include/core/SkCanvas.h"
#include "include/core/SkData.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkSurface.h"
#include "modules/skottie/include/Skottie.h"
#include "include/core/SkStream.h"
#include "include/core/SkGraphics.h"

int main() {
    // 初始化 Skia 图形系统
    SkGraphics::Init();

    // 读取 Lottie 动画 JSON 文件
    std::unique_ptr<SkStream> jsonStream = SkStream::MakeFromFile("animation.json");
    if (!jsonStream) {
        printf("Failed to open animation.json\n");
        return -1;
    }

    // 解析 Lottie 动画
    auto animation = skottie::Animation::Make(jsonStream.get());
    if (!animation) {
        printf("Failed to parse animation\n");
        return -1;
    }

    // 创建一个 Skia 画布来渲染动画
    const int width = 800;
    const int height = 600;
    auto surface = SkSurface::MakeRasterN32Premul(width, height);
    SkCanvas* canvas = surface->getCanvas();

    // 设置动画时间
    const double duration = animation->duration();
    const double fps = 60.0;
    double t = 0.0;

    while (t < duration) {
        canvas->clear(SK_ColorWHITE);

        // 渲染动画到画布
        animation->render(canvas, t / duration);

        // 这里你可以将 canvas 内容保存到文件或者显示在窗口中
        // 示例：保存到 PNG 文件
        auto image = surface->makeImageSnapshot();
        auto data = image->encodeToData();
        SkFILEWStream out("frame.png");
        out.write(data->data(), data->size());

        // 更新时间
        t += 1.0 / fps;
    }

    return 0;
}

```