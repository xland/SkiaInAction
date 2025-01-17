#pragma once
#include <windows.h>

#include <webgpu/webgpu_cpp.h>
#include <dawn/native/DawnNative.h>
#include <include/gpu/graphite/dawn/DawnBackendContext.h>
#include <include/gpu/graphite/Surface.h>
#include <include/gpu/graphite/Context.h>
#include <include/gpu/graphite/ContextOptions.h>
#include <include/gpu/graphite/dawn/DawnTypes.h>
#include <include/gpu/graphite/dawn/DawnUtils.h>
#include <include/gpu/graphite/BackendTexture.h>
#include <src/gpu/graphite/ContextOptionsPriv.h>

#include <include/core/SkCanvas.h>
#include <include/core/SkSurface.h>
#include <include/core/SkPaint.h>

#include "DisplayParams.h"

class WindowMain {
public:
	WindowMain();
	~WindowMain();
private:
	void paint(SkCanvas* canvas);
	void initPosSize();
	void configSurface();
	void createWindow();
	void initDawnDevice();
	void flush();
	sk_sp<SkSurface> getSurface();
	void initDawnInstance();
	void initDawnSurface();
	void initGraphite();
	static LRESULT CALLBACK wndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
private:
	int x, y, w, h;
	HWND hwnd;
	DisplayParams displayParams;
	wgpu::TextureFormat surfaceFormat{ wgpu::TextureFormat::BGRA8Unorm };
	wgpu::Device dawnDevice;
	std::unique_ptr<dawn::native::Instance> dawnInstance;
	wgpu::Surface dawnSurface;
	std::unique_ptr<skgpu::graphite::Context> graphiteContext;
	std::unique_ptr<skgpu::graphite::Recorder> graphiteRecorder;
};