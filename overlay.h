#pragma once

#define WIN32_MEAN_AND_LEAN
#include <Windows.h>
#include <dwmapi.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <tchar.h>
#include <vector>
#include <TlHelp32.h>
#include <string>
#include <random>
#include <d3d11.h>
#include <dxgi.h>

#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace window {
	inline HWND hwnd;
	inline HINSTANCE instance;
	inline uint32_t width, height;

	namespace directx
	{
		inline ID3D11Device* device = nullptr;
		inline ID3D11DeviceContext* context = nullptr;
		inline IDXGISwapChain* swap_chain = nullptr;
		inline ID3D11RenderTargetView* render_target_view = nullptr;
	}
	
	LRESULT WINAPI WndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
	void cleanup();
	bool create_window();
	bool create_device();
	void new_frame();
	void draw();
}

namespace overlay {
	inline HWND target;
	inline uint32_t width, height;

	bool initialize(const wchar_t* window_class, const wchar_t* window_name);
	bool scale();
	void click_through(bool click);
}
