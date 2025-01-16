#include <dawn/native/DawnNative.h>
#include <dawn/dawn_proc.h>


#include "WindowMain.h"

WindowMain::WindowMain()
{
    initPosSize();
    createWindow();
}

WindowMain::~WindowMain()
{
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

void WindowMain::configSurface()
{
    wgpu::SurfaceConfiguration surfaceConfig;
    surfaceConfig.device = fDevice;
    surfaceConfig.format = surfaceFormat;
    surfaceConfig.usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::TextureBinding |
        wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::CopyDst;
    surfaceConfig.width = w;
    surfaceConfig.height = h;
    //wgpu::PresentMode::Immediate 立即渲染会撕裂
    //wgpu::PresentMode::Fifo 渲染的帧内容会进入一个 FIFO（先进先出）队列,等待显示器的垂直同步信号（VSync）后再显示到屏幕上。
    surfaceConfig.presentMode = wgpu::PresentMode::Fifo;
    fSurface.Configure(&surfaceConfig);
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
    ShowWindow(hwnd, SW_SHOW);
}

void WindowMain::initD3D()
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
    adapterOptions.compatibilityMode = false;
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
    device = adapter.CreateDevice(&deviceDescriptor);


    wgpu::SurfaceDescriptorFromWindowsHWND surfaceChainedDesc;
    surfaceChainedDesc.hwnd = hwnd;
    surfaceChainedDesc.hinstance = GetModuleHandle(nullptr);
    wgpu::SurfaceDescriptor surfaceDesc;
    surfaceDesc.nextInChain = &surfaceChainedDesc;
    fSurface = wgpu::Instance(fInstance->Get()).CreateSurface(&surfaceDesc);
}

void WindowMain::initDawn()
{
    initDawnInstance();
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
            //window->onPaint();
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
            //window->onResize(LOWORD(lParam), HIWORD(lParam));
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
