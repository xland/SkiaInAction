#pragma once
#include <windows.h>

#include <webgpu/webgpu_cpp.h>
#include <dawn/native/DawnNative.h>
#include <include/gpu/graphite/dawn/DawnBackendContext.h>
#include <include/gpu/graphite/dawn/DawnTypes.h>
#include <include/gpu/graphite/dawn/DawnUtils.h>

#include "DisplayParams.h"

class WindowMain {
public:
	WindowMain();
	~WindowMain();
private:
	void initPosSize();
	void configSurface();
	void createWindow();
	void initD3D();
	void initDawn();
	void initDawnInstance();
	static LRESULT CALLBACK wndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
private:
	int x, y, w, h;
	wgpu::Device device;
	HWND hwnd;
	DisplayParams displayParams;
	wgpu::TextureFormat surfaceFormat{ wgpu::TextureFormat::BGRA8Unorm };
	std::unique_ptr<dawn::native::Instance> dawnInstance;
};