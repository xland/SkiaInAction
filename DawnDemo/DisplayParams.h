#pragma once
#include <include/core/SkCanvas.h>
#include <include/core/SkColorType.h>
#include <include/core/SkColorSpace.h>
#include <include/core/SkSurface.h>
#include <include/core/SkPaint.h>
#include <include/gpu/ganesh/GrDirectContext.h>

class DisplayParams {
public:
    DisplayParams();
public:
    SkColorType            fColorType;
    sk_sp<SkColorSpace>    fColorSpace;
    int                    fMSAASampleCount;
    GrContextOptions       fGrContextOptions;
    SkSurfaceProps         fSurfaceProps;
    bool                   fDisableVsync;
    bool                   fDelayDrawableAcquisition;
    bool                   fCreateProtectedNativeBackend = false;
};