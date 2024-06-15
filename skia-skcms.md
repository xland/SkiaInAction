skcms.lib 是 Skia 中的一个库，用于处理颜色管理相关的功能。SkCMS（Skia Color Management System）是一个轻量级的颜色管理系统，主要用于在不同颜色空间之间进行转换和管理颜色配置文件（ICC profiles）。

```c++
#include "skcms.h"
#include <stdio.h>

int main() {
    // 定义一个 sRGB 颜色
    skcms_ICCProfile srgb_profile;
    skcms_GetProfileFromSRGB(&srgb_profile);

    // 定义一个 Adobe RGB 颜色配置文件
    skcms_ICCProfile adobe_rgb_profile;
    skcms_Parse(
        // Adobe RGB ICC profile data
        adobe_rgb_icc_data, sizeof(adobe_rgb_icc_data),
        &adobe_rgb_profile);

    // 定义一个颜色在 sRGB 中的值
    float srgb_color[3] = {0.5f, 0.5f, 0.5f};  // 中灰色

    // 转换到 Adobe RGB
    float adobe_rgb_color[3];
    if (skcms_Transform(
            srgb_color, skcms_PixelFormat_RGB_fff, skcms_AlphaFormat_Unpremul, &srgb_profile,
            adobe_rgb_color, skcms_PixelFormat_RGB_fff, skcms_AlphaFormat_Unpremul, &adobe_rgb_profile,
            1 /* number of pixels */)) {
        printf("Converted to Adobe RGB: R=%.3f, G=%.3f, B=%.3f\n",
               adobe_rgb_color[0], adobe_rgb_color[1], adobe_rgb_color[2]);
    } else {
        printf("Color conversion failed!\n");
    }

    return 0;
}

```