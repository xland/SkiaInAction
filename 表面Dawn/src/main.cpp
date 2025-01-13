#include <windows.h>
#include <string>
#include "include/core/SkSurface.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkBitmap.h"
#include "include/gpu/ganesh/GrTypes.h"
#include "webgpu/webgpu_cpp.h"
#include "dawn/native/DawnNative.h"
#include "dawn/dawn_proc.h"
#include <vector>

int w{400}, h{400};

void surfaceWritePixels(SkSurface *surface)
{
    std::vector<SkColor> srcMem(200 * 200, 0xff00ffff);
    SkBitmap dstBitmap;
    dstBitmap.setInfo(SkImageInfo::MakeN32Premul(200, 200));
    dstBitmap.setPixels(&srcMem.front());
    surface->writePixels(dstBitmap, 100, 100);
}

wgpu::Device createDevice(wgpu::BackendType type) {
    DawnProcTable backendProcs = dawn::native::GetProcs();
    dawnProcSetProcs(&backendProcs);

    static constexpr const char* kToggles[] = {
        "allow_unsafe_apis",  // Needed for dual-source blending, BufferMapExtendedUsages.
        "use_user_defined_labels_in_backend",
        // Robustness impacts performance and is always disabled when running Graphite in Chrome,
        // so this keeps Skia's tests operating closer to real-use behavior.
        "disable_robustness",
        // Must be last to correctly respond to `fUseTintIR` option.
        "use_tint_ir",
    };
    wgpu::DawnTogglesDescriptor togglesDesc;
    togglesDesc.enabledToggleCount =
        std::size(kToggles) -
        (fDisplayParams->graphiteTestOptions()->fTestOptions.fUseTintIR ? 0 : 1);
    togglesDesc.enabledToggles = kToggles;

    wgpu::RequestAdapterOptions adapterOptions;
    adapterOptions.backendType = type;
    adapterOptions.featureLevel =
        type == wgpu::BackendType::OpenGL || type == wgpu::BackendType::OpenGLES
        ? wgpu::FeatureLevel::Compatibility
        : wgpu::FeatureLevel::Core;
    adapterOptions.nextInChain = &togglesDesc;

    std::vector<dawn::native::Adapter> adapters = fInstance->EnumerateAdapters(&adapterOptions);
    if (adapters.empty()) {
        return nullptr;
    }

    wgpu::Adapter adapter = adapters[0].Get();

    std::vector<wgpu::FeatureName> features;
    if (adapter.HasFeature(wgpu::FeatureName::MSAARenderToSingleSampled)) {
        features.push_back(wgpu::FeatureName::MSAARenderToSingleSampled);
    }
    if (adapter.HasFeature(wgpu::FeatureName::TransientAttachments)) {
        features.push_back(wgpu::FeatureName::TransientAttachments);
    }
    if (adapter.HasFeature(wgpu::FeatureName::Unorm16TextureFormats)) {
        features.push_back(wgpu::FeatureName::Unorm16TextureFormats);
    }
    if (adapter.HasFeature(wgpu::FeatureName::DualSourceBlending)) {
        features.push_back(wgpu::FeatureName::DualSourceBlending);
    }
    if (adapter.HasFeature(wgpu::FeatureName::FramebufferFetch)) {
        features.push_back(wgpu::FeatureName::FramebufferFetch);
    }
    if (adapter.HasFeature(wgpu::FeatureName::BufferMapExtendedUsages)) {
        features.push_back(wgpu::FeatureName::BufferMapExtendedUsages);
    }
    if (adapter.HasFeature(wgpu::FeatureName::TextureCompressionETC2)) {
        features.push_back(wgpu::FeatureName::TextureCompressionETC2);
    }
    if (adapter.HasFeature(wgpu::FeatureName::TextureCompressionBC)) {
        features.push_back(wgpu::FeatureName::TextureCompressionBC);
    }
    if (adapter.HasFeature(wgpu::FeatureName::R8UnormStorage)) {
        features.push_back(wgpu::FeatureName::R8UnormStorage);
    }
    if (adapter.HasFeature(wgpu::FeatureName::DawnLoadResolveTexture)) {
        features.push_back(wgpu::FeatureName::DawnLoadResolveTexture);
    }
    if (adapter.HasFeature(wgpu::FeatureName::DawnPartialLoadResolveTexture)) {
        features.push_back(wgpu::FeatureName::DawnPartialLoadResolveTexture);
    }
    if (adapter.HasFeature(wgpu::FeatureName::TimestampQuery)) {
        features.push_back(wgpu::FeatureName::TimestampQuery);
    }
    if (adapter.HasFeature(wgpu::FeatureName::DawnTexelCopyBufferRowAlignment)) {
        features.push_back(wgpu::FeatureName::DawnTexelCopyBufferRowAlignment);
    }

    wgpu::DeviceDescriptor deviceDescriptor;
    deviceDescriptor.requiredFeatures = features.data();
    deviceDescriptor.requiredFeatureCount = features.size();
    deviceDescriptor.nextInChain = &togglesDesc;
    deviceDescriptor.SetDeviceLostCallback(
        wgpu::CallbackMode::AllowSpontaneous,
        [](const wgpu::Device&, wgpu::DeviceLostReason reason, const char* message) {
            if (reason != wgpu::DeviceLostReason::Destroyed &&
                reason != wgpu::DeviceLostReason::InstanceDropped) {
                SK_ABORT("Device lost: %s\n", message);
            }
        });
    deviceDescriptor.SetUncapturedErrorCallback(
        [](const wgpu::Device&, wgpu::ErrorType, const char* message) {
            SkDebugf("Device error: %s\n", message);
            SkASSERT(false);
        });

    wgpu::DawnTogglesDescriptor deviceTogglesDesc;

    if (fDisplayParams->graphiteTestOptions()->fTestOptions.fDisableTintSymbolRenaming) {
        static constexpr const char* kOptionalDeviceToggles[] = {
            "disable_symbol_renaming",
        };
        deviceTogglesDesc.enabledToggleCount = std::size(kOptionalDeviceToggles);
        deviceTogglesDesc.enabledToggles = kOptionalDeviceToggles;

        // Insert the toggles descriptor ahead of any existing entries in the chain that might have
        // been added above.
        deviceTogglesDesc.nextInChain = deviceDescriptor.nextInChain;
        deviceDescriptor.nextInChain = &deviceTogglesDesc;
    }

    auto device = adapter.CreateDevice(&deviceDescriptor);
    if (!device) {
        return nullptr;
    }

    return device;
}



void paint(const HWND hWnd)
{
    if (w <= 0 || h <= 0)
        return;
    SkColor *surfaceMemory = new SkColor[w * h]{0xff000000};
    SkImageInfo info = SkImageInfo::MakeN32Premul(w, h);
    sk_sp<SkSurface> surface = SkSurfaces::WrapPixels(info, surfaceMemory, w * 4);
    surfaceWritePixels(surface.get());

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

    auto device = createDevice(wgpu::BackendType::D3D12);
    if (!device) {
        SkASSERT(device);
        return;
    }

    wgpu::SurfaceDescriptorFromWindowsHWND surfaceChainedDesc;
    surfaceChainedDesc.hwnd = hwnd;
    surfaceChainedDesc.hinstance = GetModuleHandle(nullptr);
    wgpu::SurfaceDescriptor surfaceDesc;
    surfaceDesc.nextInChain = &surfaceChainedDesc;

    auto surface = wgpu::Instance(fInstance->Get()).CreateSurface(&surfaceDesc);
    if (!surface) {
        SkASSERT(false);
        return false;
    }

    fDevice = std::move(device);
    fSurface = std::move(surface);
    configureSurface();



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