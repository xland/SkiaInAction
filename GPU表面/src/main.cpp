#include <windows.h>
#include <string>
#include "include/core/SkColorSpace.h"
#include "include/core/SkSurface.h"
#include "include/core/SkCanvas.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "include/gpu/ganesh/gl/GrGLBackendSurface.h"
#include "include/gpu/ganesh/gl/GrGLDirectContext.h"
#include "include/gpu/gl/GrGLInterface.h"
#include "include/gpu/ganesh/gl/egl/GrGLMakeEGLInterface.h"
//#include "include/gpu/ganesh/gl/mac/GrGLMakeMacInterface.h"
#include "tools/gpu/gl/win/SkWGL.h"

#include "include/private/base/SkTDArray.h"

#include <GL/gl.h>
#include <vector>
HWND hwnd;
int w{400}, h{400};
sk_sp<GrDirectContext> ctx;
int stencilBits{ 0 };


static void get_pixel_formats_to_try(HDC dc, const SkWGLExtensions& extensions,
    bool doubleBuffered, int msaaSampleCount, bool deepColor,
    int formatsToTry[2]) {
    auto appendAttr = [](SkTDArray<int>& attrs, int attr, int value) {
        attrs.push_back(attr);
        attrs.push_back(value);
        };

    SkTDArray<int> iAttrs;
    appendAttr(iAttrs, SK_WGL_DRAW_TO_WINDOW, TRUE);
    appendAttr(iAttrs, SK_WGL_DOUBLE_BUFFER, (doubleBuffered ? TRUE : FALSE));
    appendAttr(iAttrs, SK_WGL_ACCELERATION, SK_WGL_FULL_ACCELERATION);
    appendAttr(iAttrs, SK_WGL_SUPPORT_OPENGL, TRUE);
    if (deepColor) {
        appendAttr(iAttrs, SK_WGL_RED_BITS, 10);
        appendAttr(iAttrs, SK_WGL_GREEN_BITS, 10);
        appendAttr(iAttrs, SK_WGL_BLUE_BITS, 10);
        appendAttr(iAttrs, SK_WGL_ALPHA_BITS, 2);
    }
    else {
        appendAttr(iAttrs, SK_WGL_COLOR_BITS, 24);
        appendAttr(iAttrs, SK_WGL_ALPHA_BITS, 8);
    }
    appendAttr(iAttrs, SK_WGL_STENCIL_BITS, 8);

    float fAttrs[] = { 0, 0 };

    // Get a MSAA format if requested and possible.
    if (msaaSampleCount > 0 &&
        extensions.hasExtension(dc, "WGL_ARB_multisample")) {
        SkTDArray<int> msaaIAttrs = iAttrs;
        appendAttr(msaaIAttrs, SK_WGL_SAMPLE_BUFFERS, TRUE);
        appendAttr(msaaIAttrs, SK_WGL_SAMPLES, msaaSampleCount);
        appendAttr(msaaIAttrs, 0, 0);
        unsigned int num;
        int formats[64];
        extensions.choosePixelFormat(dc, msaaIAttrs.begin(), fAttrs, 64, formats, &num);
        num = std::min(num, 64U);
        formatsToTry[0] = extensions.selectFormat(formats, num, dc, msaaSampleCount);
    }

    // Get a non-MSAA format
    int* format = -1 == formatsToTry[0] ? &formatsToTry[0] : &formatsToTry[1];
    unsigned int num;
    appendAttr(iAttrs, 0, 0);
    extensions.choosePixelFormat(dc, iAttrs.begin(), fAttrs, 1, format, &num);
}

static HGLRC create_gl_context(HDC dc, const SkWGLExtensions& extensions,
    SkWGLContextRequest contextType, HGLRC shareContext) {
    HDC prevDC = wglGetCurrentDC();
    HGLRC prevGLRC = wglGetCurrentContext();

    HGLRC glrc = nullptr;
    if (kGLES_SkWGLContextRequest == contextType) {
        static const int glesAttribs[] = {
            SK_WGL_CONTEXT_MAJOR_VERSION, 3,
            SK_WGL_CONTEXT_MINOR_VERSION, 0,
            SK_WGL_CONTEXT_PROFILE_MASK,  SK_WGL_CONTEXT_ES2_PROFILE_BIT,
            0,
        };
        glrc = extensions.createContextAttribs(dc, shareContext, glesAttribs);
    }
    else {
        if (kGLPreferCoreProfile_SkWGLContextRequest == contextType &&
            extensions.hasExtension(dc, "WGL_ARB_create_context")) {
            static const int kCoreGLVersions[] = {
                4, 3,
                4, 2,
                4, 1,
                4, 0,
                3, 3,
                3, 2,
            };
            int coreProfileAttribs[] = {
                SK_WGL_CONTEXT_MAJOR_VERSION, -1,
                SK_WGL_CONTEXT_MINOR_VERSION, -1,
                SK_WGL_CONTEXT_PROFILE_MASK,  SK_WGL_CONTEXT_CORE_PROFILE_BIT,
                0,
            };
            for (size_t v = 0; v < std::size(kCoreGLVersions) / 2; ++v) {
                coreProfileAttribs[1] = kCoreGLVersions[2 * v];
                coreProfileAttribs[3] = kCoreGLVersions[2 * v + 1];
                glrc = extensions.createContextAttribs(dc, shareContext, coreProfileAttribs);
                if (glrc) {
                    break;
                }
            }
        }
    }

    if (nullptr == glrc) {
        glrc = wglCreateContext(dc);
    }
    SkASSERT(glrc);

    wglMakeCurrent(prevDC, prevGLRC);

    return glrc;
}

void initSurface() {

    HDC dc = GetDC(hwnd);
    //auto hglrc = SkCreateWGLContext(dc, 8, false, kGLPreferCompatibilityProfile_SkWGLContextRequest);
    SkWGLExtensions extensions;
    BOOL set = FALSE;
    int pixelFormatsToTry[] = { -1, -1 };
    get_pixel_formats_to_try(dc, extensions, true, 8, false, pixelFormatsToTry);
    for (size_t f = 0;
        !set && -1 != pixelFormatsToTry[f] && f < std::size(pixelFormatsToTry);
        ++f) {
        PIXELFORMATDESCRIPTOR pfd;
        DescribePixelFormat(dc, pixelFormatsToTry[f], sizeof(pfd), &pfd);
        set = SetPixelFormat(dc, pixelFormatsToTry[f], &pfd);
    }
    if (!set) {
        return;
    }
    auto hglrc = create_gl_context(dc, extensions, kGLPreferCompatibilityProfile_SkWGLContextRequest, nullptr);




    wglMakeCurrent(dc, hglrc);
    glClearStencil(0);
    glClearColor(0, 0, 0, 0);
    glStencilMask(0xffffffff);
    glClear(GL_STENCIL_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    int pixelFormat = GetPixelFormat(dc);
    PIXELFORMATDESCRIPTOR pfd;
    DescribePixelFormat(dc, pixelFormat, sizeof(pfd), &pfd);
    stencilBits = pfd.cStencilBits;


    auto interfaceObj = GrGLMakeNativeInterface();
    ctx = GrDirectContexts::MakeGL(interfaceObj);
    glViewport(0, 0, w, h);
    GrBackendRenderTarget target = GrBackendRenderTargets::MakeGL(w,h,1,8,GrGLFramebufferInfo{ 0, GL_RGBA8 });
    static const SkSurfaceProps surfaceProps;
    auto surface = SkSurfaces::WrapBackendRenderTarget(ctx.get(), target, kBottomLeft_GrSurfaceOrigin,
        kRGBA_8888_SkColorType,
        nullptr,
        &surfaceProps);
}

void paint(const HWND hWnd)
{
    if (w <= 0 || h <= 0)
        return;
    SkColor *surfaceMemory = new SkColor[w * h]{0xff000000};
    SkImageInfo info = SkImageInfo::MakeN32Premul(w, h);
    auto surface = SkSurfaces::WrapPixels(info, surfaceMemory, w * 4);

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
    hwnd = CreateWindow(clsName.c_str(), clsName.c_str(), WS_OVERLAPPEDWINDOW,
                             CW_USEDEFAULT, CW_USEDEFAULT, w, h,
                             nullptr, nullptr, hinstance, nullptr);
    ShowWindow(hwnd, SW_SHOW);
    initSurface();
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