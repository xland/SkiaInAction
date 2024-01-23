#include "GLContext.h"
#include <GL/gl.h>
#include "src/gpu/ganesh/gl/GrGLGpu.h"
#include "src/utils/win/SkWGL.h"
#include "include/gpu/gl/GrGLInterface.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/ganesh/gl/GrGLBackendSurface.h"

#include "include/gpu/ganesh/gl/GrGLDirectContext.h"

#include "include/gpu/ganesh/SkSurfaceGanesh.h"

GLContext::GLContext()
{
    fMSAASampleCount = GrNextPow2(fMSAASampleCount);
    initializeContext();
}

GLContext::~GLContext()
{
}

void GLContext::initializeContext()
{
    fBackendContext = this->onInitializeContext();
    fContext = GrDirectContexts::MakeGL(fBackendContext, fGrContextOptions);
    if (!fContext && fMSAASampleCount > 1) {
        fMSAASampleCount /= 2;
        this->initializeContext();
        return;
    }
}

sk_sp<const GrGLInterface> GLContext::onInitializeContext()
{
    HDC dc = GetDC(fHWND);
    fHGLRC = SkCreateWGLContext(dc, fMSAASampleCount, false /* deepColor */,
        kGLPreferCompatibilityProfile_SkWGLContextRequest);
    if (nullptr == fHGLRC) {
        return nullptr;
    }

    SkWGLExtensions extensions;
    if (extensions.hasExtension(dc, "WGL_EXT_swap_control")) {
        extensions.swapInterval(fDisableVsync ? 0 : 1);
    }

    // Look to see if RenderDoc is attached. If so, re-create the context with a core profile
    if (wglMakeCurrent(dc, fHGLRC)) {
        auto interfaceGL = GrGLMakeNativeInterface();
        bool renderDocAttached = interfaceGL->hasExtension("GL_EXT_debug_tool");
        interfaceGL.reset(nullptr);
        if (renderDocAttached) {
            wglDeleteContext(fHGLRC);
            fHGLRC = SkCreateWGLContext(dc, fMSAASampleCount, false /* deepColor */,
                kGLPreferCoreProfile_SkWGLContextRequest);
            if (nullptr == fHGLRC) {
                return nullptr;
            }
        }
    }

    if (wglMakeCurrent(dc, fHGLRC)) {
        glClearStencil(0);
        glClearColor(0, 0, 0, 0);
        glStencilMask(0xffffffff);
        glClear(GL_STENCIL_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

        // use DescribePixelFormat to get the stencil and color bit depth.
        int pixelFormat = GetPixelFormat(dc);
        PIXELFORMATDESCRIPTOR pfd;
        DescribePixelFormat(dc, pixelFormat, sizeof(pfd), &pfd);
        fStencilBits = pfd.cStencilBits;

        // Get sample count if the MSAA WGL extension is present
        if (extensions.hasExtension(dc, "WGL_ARB_multisample")) {
            static const int kSampleCountAttr = SK_WGL_SAMPLES;
            extensions.getPixelFormatAttribiv(dc,
                pixelFormat,
                0,
                1,
                &kSampleCountAttr,
                &fSampleCount);
            fSampleCount = std::max(fSampleCount, 1);
        }
        else {
            fSampleCount = 1;
        }

        RECT rect;
        GetClientRect(fHWND, &rect);
        fWidth = rect.right - rect.left;
        fHeight = rect.bottom - rect.top;
        glViewport(0, 0, fWidth, fHeight);
    }
    return GrGLMakeNativeInterface();
}

sk_sp<SkSurface> GLContext::GetSurface() {
    if (nullptr == fSurface) {
        if (fContext) {
            GrGLint buffer;
            GR_GL_CALL(fBackendContext.get(), GetIntegerv(GR_GL_FRAMEBUFFER_BINDING, &buffer));

            GrGLFramebufferInfo fbInfo;
            fbInfo.fFBOID = buffer;
            fbInfo.fFormat = GR_GL_RGBA8;
            fbInfo.fProtected = skgpu::Protected(fCreateProtectedNativeBackend);

            auto backendRT = GrBackendRenderTargets::MakeGL(fWidth,
                fHeight,
                fSampleCount,
                fStencilBits,
                fbInfo);

            fSurface = SkSurfaces::WrapBackendRenderTarget(fContext.get(),
                backendRT,
                kBottomLeft_GrSurfaceOrigin,
                kRGBA_8888_SkColorType,
                fColorSpace,
                &fSurfaceProps);
        }
    }

    return fSurface;
}