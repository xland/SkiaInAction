#include <windows.h>
#include <include/core/SkCanvas.h>
#include <include/core/SkSurface.h>
#include <include/core/SkPaint.h>
#include <include/core/SkExecutor.h>
#include <include/core/SkTiledImageUtils.h>
#include <src/core/SkLRUCache.h>
#include <webgpu/webgpu_cpp.h>
#include <dawn/native/DawnNative.h>
#include <dawn/dawn_proc.h>
#include <include/gpu/ganesh/GrDirectContext.h>
#include <include/gpu/graphite/BackendTexture.h>
#include <include/gpu/graphite/Context.h>
#include <include/gpu/graphite/ContextOptions.h>
#include <include/gpu/graphite/GraphiteTypes.h>
#include <include/gpu/graphite/Recorder.h>
#include <include/gpu/graphite/Recording.h>
#include <include/gpu/graphite/Surface.h>
#include <include/gpu/graphite/dawn/DawnBackendContext.h>
#include <include/gpu/graphite/dawn/DawnTypes.h>
#include <include/gpu/graphite/dawn/DawnUtils.h>
#include <include/gpu/graphite/ImageProvider.h>
#include <include/gpu/ganesh/SkImageGanesh.h>
#include <include/gpu/graphite/Image.h>
#include <src/gpu/graphite/ContextOptionsPriv.h>
#include "DisplayParams.h"
#include "TestingImageProvider.h"




int w{ 800 }, h{ 600 };
HWND hwnd;
wgpu::TextureFormat fSurfaceFormat = wgpu::TextureFormat::BGRA8Unorm;
std::unique_ptr<dawn::native::Instance> fInstance;
DisplayParams fDisplayParams;
wgpu::Device fDevice;
wgpu::Surface fSurface;
std::unique_ptr<skgpu::graphite::Context> fGraphiteContext;
std::unique_ptr<skgpu::graphite::Recorder> fGraphiteRecorder;

void initDisplayParams() {
    fDisplayParams.fMSAASampleCount = 1;
    static std::unique_ptr<SkExecutor> gGpuExecutor = SkExecutor::MakeFIFOThreadPool(2);
    fDisplayParams.fGrContextOptions.fExecutor = gGpuExecutor.get();
    fDisplayParams.fGrContextOptions.fAllowPathMaskCaching = true;
    //displayParams.fGrContextOptions.fGpuPathRenderers = collect_gpu_path_renderers_from_flags();  //todo
    fDisplayParams.fGrContextOptions.fDisableDriverCorrectnessWorkarounds = false;
    fDisplayParams.fGrContextOptions.fReduceOpsTaskSplitting = GrContextOptions::Enable::kYes;
    fDisplayParams.fGrContextOptions.fAllowMSAAOnNewIntel = false;
    fDisplayParams.fGrContextOptions.fShaderCacheStrategy = GrContextOptions::ShaderCacheStrategy::kSkSL;
    fDisplayParams.fGrContextOptions.fSuppressPrints = true;
    fDisplayParams.fGrContextOptions.fSupportBilerpFromGlyphAtlas = true;
    fDisplayParams.fCreateProtectedNativeBackend = false;
}
void initD3D() {
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
    togglesDesc.enabledToggleCount = std::size(kToggles) - 1;
    togglesDesc.enabledToggles = kToggles;
    wgpu::RequestAdapterOptions adapterOptions;
    adapterOptions.backendType = wgpu::BackendType::D3D12;
    adapterOptions.compatibilityMode = false;
    adapterOptions.nextInChain = &togglesDesc;
    std::vector<dawn::native::Adapter> adapters = fInstance->EnumerateAdapters(&adapterOptions);
    if (adapters.empty()) {
        return;
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
    fDevice = adapter.CreateDevice(&deviceDescriptor);
    wgpu::SurfaceDescriptorFromWindowsHWND surfaceChainedDesc;
    surfaceChainedDesc.hwnd = hwnd;
    surfaceChainedDesc.hinstance = GetModuleHandle(nullptr);
    wgpu::SurfaceDescriptor surfaceDesc;
    surfaceDesc.nextInChain = &surfaceChainedDesc;
    fSurface = wgpu::Instance(fInstance->Get()).CreateSurface(&surfaceDesc);
    wgpu::SurfaceConfiguration surfaceConfig;
    surfaceConfig.device = fDevice;
    surfaceConfig.format = fSurfaceFormat;
    surfaceConfig.usage = wgpu::TextureUsage::RenderAttachment |
        wgpu::TextureUsage::TextureBinding |
        wgpu::TextureUsage::CopySrc |
        wgpu::TextureUsage::CopyDst;
    surfaceConfig.width = w;
    surfaceConfig.height = h;
    surfaceConfig.presentMode = fDisplayParams.fDisableVsync ? wgpu::PresentMode::Immediate : wgpu::PresentMode::Fifo;
    fSurface.Configure(&surfaceConfig);
}
void initDAWN() {    
    WGPUInstanceDescriptor desc{};
    // need for WaitAny with timeout > 0
    desc.features.timedWaitAnyEnable = true;
    fInstance = std::make_unique<dawn::native::Instance>(&desc);
	initD3D();
    skgpu::graphite::DawnBackendContext backendContext;
    backendContext.fInstance = wgpu::Instance(fInstance->Get());
    backendContext.fDevice = fDevice;
    backendContext.fQueue = fDevice.GetQueue();
    skgpu::graphite::ContextOptions fContextOptions;
    fGraphiteContext = skgpu::graphite::ContextFactory::MakeDawn(backendContext, fContextOptions);
    if (!fGraphiteContext) {
        SkASSERT(false);
        return;
    }
    skgpu::graphite::RecorderOptions options;
    options.fImageProvider.reset(new TestingImageProvider);
    fGraphiteRecorder = fGraphiteContext->makeRecorder(options);
}
void paint(const HWND hWnd)
{
    wgpu::SurfaceTexture surfaceTexture;
    fSurface.GetCurrentTexture(&surfaceTexture);
    SkASSERT(surfaceTexture.texture);
    auto texture = surfaceTexture.texture;
    skgpu::graphite::DawnTextureInfo info(/*sampleCount=*/1,
        skgpu::Mipmapped::kNo,
        fSurfaceFormat,
        texture.GetUsage(),
        wgpu::TextureAspect::All);
    auto backendTex = skgpu::graphite::BackendTextures::MakeDawn(texture.Get());
    auto surface = SkSurfaces::WrapBackendTexture(fGraphiteRecorder.get(),
        backendTex,
        kBGRA_8888_SkColorType,
        fDisplayParams.fColorSpace,
        &fDisplayParams.fSurfaceProps);  
    auto canvas = surface->getCanvas();
    SkPaint paint;
    paint.setColor(SK_ColorRED);
    SkRect rect = SkRect::MakeXYWH(w - 150, h - 150, 140, 140);
    canvas->drawRect(rect, paint);
    //->flushAndSubmit(backbuffer.get(), GrSyncCpu::kNo);
    if (fGraphiteContext) {
        SkASSERT(fGraphiteRecorder);
        std::unique_ptr<skgpu::graphite::Recording> recording = fGraphiteRecorder->snap();
        if (recording) {
            skgpu::graphite::InsertRecordingInfo info;
            info.fRecording = recording.get();
            fGraphiteContext->insertRecording(info);
            fGraphiteContext->submit(skgpu::graphite::SyncToCpu::kNo);
        }
    }
    fSurface.Present();
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
    std::wstring clsName{ L"DrawInWindow" };
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
    initDisplayParams();
	initDAWN();
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