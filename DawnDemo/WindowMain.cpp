#include <dawn/native/DawnNative.h>
#include <dawn/dawn_proc.h>
#include <include/gpu/graphite/Context.h>
#include <src/gpu/graphite/ContextPriv.h>
#include <thread>

#include "WindowMain.h"

//void debounce(std::function<void()> func, std::chrono::milliseconds delay) {
//    static std::atomic<bool> isWaiting{ false };
//    static std::thread timerThread;
//
//    if (isWaiting.exchange(true)) {
//        return; // 如果已经有一个定时器正在等待，直接返回
//    }
//
//    if (timerThread.joinable()) {
//        timerThread.join(); // 确保前一个线程结束
//    }
//
//    timerThread = std::thread([func, delay]() {
//        std::this_thread::sleep_for(delay);
//        func();
//        isWaiting.store(false);
//        });
//}



WindowMain::WindowMain()
{
    initPosSize();
    createWindow();

    initDawnInstance();
    initDawnDevice();
    initDawnSurface();
    configSurface();
    initGraphite();


    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);
}

WindowMain::~WindowMain()
{
    graphiteRecorder = nullptr;
    graphiteContext = nullptr;
    dawnSurface = nullptr;
    dawnDevice = nullptr;
}

void WindowMain::paint(SkCanvas* canvas)
{
    canvas->clear(0xFFFFFFFF);
    SkPaint paint;
    paint.setColor(SK_ColorRED);
    SkRect rect = SkRect::MakeXYWH(w - 150, h - 150, 140, 140);
    canvas->drawRect(rect, paint);
}

void WindowMain::initPosSize()
{
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);
    w = 800;
    h = 600;
    x = (screenWidth - w) / 2;
    y = (screenHeight - h) / 2;
}



void WindowMain::createWindow()
{
    static bool isWcexReg = false;
    static const TCHAR clsName[] = L"SkiaApp";
    static WNDCLASSEX wcex;
    auto hinstance = GetModuleHandle(NULL);
    if (!isWcexReg) {
        wcex.cbSize = sizeof(WNDCLASSEX);
        wcex.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
        wcex.lpfnWndProc = &WindowMain::wndProc;
        wcex.cbClsExtra = 0;
        wcex.cbWndExtra = 0;
        wcex.hInstance = hinstance;
        wcex.hIcon = LoadIcon(hinstance, IDI_APPLICATION);
        wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        wcex.lpszMenuName = nullptr;
        wcex.lpszClassName = clsName;
        wcex.hIconSm = LoadIcon(hinstance, IDI_APPLICATION);
        if (!RegisterClassEx(&wcex)) {
            return;
        }
        isWcexReg = true;
    }
    hwnd = CreateWindowEx(NULL, clsName, clsName, WS_OVERLAPPEDWINDOW,
        x, y, w, h, nullptr, nullptr, hinstance, nullptr);
    SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)this);
}
void WindowMain::configSurface()
{    
    wgpu::SurfaceConfiguration config;
    config.device = dawnDevice;
    config.format = surfaceFormat;
    config.usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::TextureBinding |
        wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::CopyDst;
    config.width = w;
    config.height = h;
    //wgpu::PresentMode::Immediate 立即渲染会撕裂
    //wgpu::PresentMode::Fifo 渲染的帧内容会进入一个 FIFO（先进先出）队列,等待显示器的垂直同步信号（VSync）后再显示到屏幕上。
    config.presentMode = wgpu::PresentMode::Fifo;
    dawnSurface.Configure(&config);
}
void WindowMain::initDawnDevice()
{
    DawnProcTable backendProcs = dawn::native::GetProcs();
    dawnProcSetProcs(&backendProcs);
    static constexpr const char* kToggles[] = {
        "allow_unsafe_apis",
        "use_user_defined_labels_in_backend",
        "disable_robustness", //禁用这玩意儿，提升性能
        "use_tint_ir",
    };
    wgpu::DawnTogglesDescriptor togglesDesc;
    togglesDesc.enabledToggleCount = std::size(kToggles) - 1;
    togglesDesc.enabledToggles = kToggles;
    wgpu::RequestAdapterOptions adapterOptions;
    adapterOptions.backendType = wgpu::BackendType::D3D11;
    adapterOptions.featureLevel = wgpu::FeatureLevel::Core;
    adapterOptions.nextInChain = &togglesDesc;
    std::vector<dawn::native::Adapter> adapters = dawnInstance->EnumerateAdapters(&adapterOptions);
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
    dawnDevice = adapter.CreateDevice(&deviceDescriptor);
}
void WindowMain::flush()
{
    std::unique_ptr<skgpu::graphite::Recording> recording = graphiteRecorder->snap();
    if (recording) {
        skgpu::graphite::InsertRecordingInfo info;
        info.fRecording = recording.get();
        graphiteContext->insertRecording(info);
        graphiteContext->submit(skgpu::graphite::SyncToCpu::kNo);
    }
    dawnSurface.Present();
}
sk_sp<SkSurface> WindowMain::getSurface()
{
    wgpu::SurfaceTexture surfaceTexture;
    dawnSurface.GetCurrentTexture(&surfaceTexture);
    auto texture = surfaceTexture.texture;
    skgpu::graphite::DawnTextureInfo info(1,skgpu::Mipmapped::kNo, 
        surfaceFormat, wgpu::TextureUsage::None, wgpu::TextureAspect::All);
    auto backendTex = skgpu::graphite::BackendTextures::MakeDawn(texture.Get());
    auto surface = SkSurfaces::WrapBackendTexture(graphiteRecorder.get(),backendTex, 
        kBGRA_8888_SkColorType, displayParams.fColorSpace, &displayParams.fSurfaceProps);
    return surface;
}
void WindowMain::initDawnSurface()
{
    wgpu::SurfaceDescriptorFromWindowsHWND surfaceChainedDesc;
    surfaceChainedDesc.hwnd = hwnd;
    surfaceChainedDesc.hinstance = GetModuleHandle(nullptr);
    wgpu::SurfaceDescriptor surfaceDesc;
    surfaceDesc.nextInChain = &surfaceChainedDesc;
    dawnSurface = wgpu::Instance(dawnInstance->Get()).CreateSurface(&surfaceDesc);
}
void WindowMain::initGraphite()
{
    skgpu::graphite::DawnBackendContext backendContext;
    backendContext.fInstance = wgpu::Instance(dawnInstance->Get());
    backendContext.fDevice = dawnDevice;
    backendContext.fQueue = dawnDevice.GetQueue();
    skgpu::graphite::ContextOptions fContextOptions;
    graphiteContext = skgpu::graphite::ContextFactory::MakeDawn(backendContext, fContextOptions);
    if (!graphiteContext) {
        SkASSERT(false);
        return;
    }
    graphiteRecorder = graphiteContext->makeRecorder();
}
void WindowMain::initDawnInstance()
{
    WGPUInstanceDescriptor desc{};
    desc.features.timedWaitAnyEnable = true;
    dawnInstance = std::make_unique<dawn::native::Instance>(&desc);
}

LRESULT WindowMain::wndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    auto win = (WindowMain*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
    switch (msg)
    {
        case WM_PAINT:{
            PAINTSTRUCT ps;
            BeginPaint(hWnd, &ps);
            auto surface = win->getSurface();
            win->paint(surface->getCanvas());
            win->flush();
            EndPaint(hWnd, &ps);
            return 0;
        }
        case WM_ACTIVATE: {
            // disable/enable rendering here, depending on wParam != WA_INACTIVE
            break;
    }
        case WM_SIZE: {
            win->w = LOWORD(lParam);
            win->h = HIWORD(lParam);
            win->configSurface();
            RedrawWindow(hWnd, nullptr, nullptr, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
            //auto surface = win->getSurface();
            //win->paint(surface->getCanvas());
            //win->flush();
            //UpdateWindow(hWnd);
            //debounce([]() {
            //        SystemParametersInfo(SPI_SETDESKWALLPAPER, 0, nullptr, SPIF_UPDATEINIFILE);
            //    }, std::chrono::milliseconds(500));
            return 0;
        }
        case WM_DESTROY:
        {
            SetWindowLongPtr(hWnd, GWLP_USERDATA, 0);
            UnregisterClass(L"SkiaApp", nullptr);
            PostQuitMessage(0);
            return 0;
        }
        default: {
            break;
        }
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}
