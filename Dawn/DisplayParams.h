#pragma once
#include <include/core/SkCanvas.h>
#include <include/core/SkColorType.h>
#include <include/core/SkColorSpace.h>
#include <include/core/SkSurface.h>
#include <include/core/SkPaint.h>
#include <include/gpu/ganesh/GrDirectContext.h>

struct DisplayParams {
    DisplayParams()
        : fColorType(kN32_SkColorType)
        , fColorSpace(nullptr)
        , fMSAASampleCount(1)
        , fSurfaceProps(0, kRGB_H_SkPixelGeometry)
        , fDisableVsync(false)
        , fDelayDrawableAcquisition(false)
        , fCreateProtectedNativeBackend(false)
    {
    }
    SkColorType            fColorType;
    sk_sp<SkColorSpace>    fColorSpace;
    int                    fMSAASampleCount;
    GrContextOptions       fGrContextOptions;
    SkSurfaceProps         fSurfaceProps;
    bool                   fDisableVsync;
    bool                   fDelayDrawableAcquisition;
    bool                   fCreateProtectedNativeBackend = false;
};