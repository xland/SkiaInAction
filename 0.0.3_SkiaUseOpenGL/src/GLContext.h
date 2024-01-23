#pragma once
#include <Windows.h>
#include "include/core/SkSurface.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkSurfaceProps.h"
#include "include/gpu/GrContextOptions.h"
#include "include/gpu/gl/GrGLInterface.h"

class GLContext
{
public:
	GLContext();
	~GLContext();
    sk_sp<SkSurface> GetSurface();
protected:
private:

    void initializeContext();
    sk_sp<const GrGLInterface> onInitializeContext();


    sk_sp<SkSurface>           fSurface;
    sk_sp<GrDirectContext> fContext;
    sk_sp<const GrGLInterface> fBackendContext;


    HWND fHWND;
    HGLRC fHGLRC;
    int               fStencilBits = 0;
    int               fSampleCount = 1;
    int               fWidth;
    int               fHeight;

    SkColorType            fColorType{ kN32_SkColorType };
    sk_sp<SkColorSpace>    fColorSpace{ nullptr };
    int                    fMSAASampleCount{1};
    GrContextOptions       fGrContextOptions;
    SkSurfaceProps         fSurfaceProps{ 0, kRGB_H_SkPixelGeometry };
    bool                   fDisableVsync{false};
    bool                   fDelayDrawableAcquisition{false};
    bool                   fEnableBinaryArchive{false};
    bool                   fCreateProtectedNativeBackend{false};
};