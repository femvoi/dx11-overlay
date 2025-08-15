#include "overlay.h"

extern "C" {
    __declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
    __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

LRESULT WINAPI window::WndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) {
    if (ImGui_ImplWin32_WndProcHandler(hWnd, Msg, wParam, lParam))
        return true;

	switch (Msg)
	{
	case WM_SIZE:
		return 0;

	case WM_SYSCOMMAND:
		if ((wParam & 0xfff0) == SC_KEYMENU)
			return 0;
		break;

	case WM_DESTROY:
		::PostQuitMessage(0);
		return 0;

	default:
		break;
	}
	return ::DefWindowProc(hWnd, Msg, wParam, lParam);
}


bool window::create_device() {
    if (!hwnd || width <= 0 || height <= 0)
        return false;

    DXGI_SWAP_CHAIN_DESC sd = {};
    sd.BufferCount = 1;
    sd.BufferDesc.Width = width;
    sd.BufferDesc.Height = height;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 1000;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = window::hwnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

    const D3D_FEATURE_LEVEL featureLevelArray[] = { D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0 };
    UINT create_device_flags = 0;
    D3D_FEATURE_LEVEL feature_level;

    directx::swap_chain = nullptr;
    directx::device = nullptr;
    directx::context = nullptr;

    IDXGIFactory* dxgi_factory = nullptr;
    HRESULT hr = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&dxgi_factory);
    if (FAILED(hr))
        return false;

    IDXGIAdapter* selected_adapter = nullptr;
    SIZE_T max_dedicated_video_memory = 0;
    IDXGIAdapter* adapter = nullptr;
    UINT i = 0;

    while (dxgi_factory->EnumAdapters(i, &adapter) != DXGI_ERROR_NOT_FOUND) {
        DXGI_ADAPTER_DESC adapter_desc;
        adapter->GetDesc(&adapter_desc);
        if (adapter_desc.DedicatedVideoMemory > max_dedicated_video_memory) {
            max_dedicated_video_memory = adapter_desc.DedicatedVideoMemory;
            if (selected_adapter)
                selected_adapter->Release();
            selected_adapter = adapter;
        }
        else {
            adapter->Release();
        }
        ++i;
    }

    if (selected_adapter) {
        hr = D3D11CreateDeviceAndSwapChain(
            selected_adapter,
            D3D_DRIVER_TYPE_UNKNOWN,
            nullptr,
            create_device_flags,
            featureLevelArray,
            _countof(featureLevelArray),
            D3D11_SDK_VERSION,
            &sd,
            &directx::swap_chain,
            &directx::device,
            &feature_level,
            &directx::context
        );
        selected_adapter->Release();
    }
    else {
        hr = D3D11CreateDeviceAndSwapChain(
            nullptr,
            D3D_DRIVER_TYPE_HARDWARE,
            nullptr,
            create_device_flags,
            featureLevelArray,
            _countof(featureLevelArray),
            D3D11_SDK_VERSION,
            &sd,
            &directx::swap_chain,
            &directx::device,
            &feature_level,
            &directx::context
        );
    }

    dxgi_factory->Release();

    if (FAILED(hr)) {
        hr = D3D11CreateDeviceAndSwapChain(
            nullptr,
            D3D_DRIVER_TYPE_WARP,
            nullptr,
            create_device_flags,
            featureLevelArray,
            _countof(featureLevelArray),
            D3D11_SDK_VERSION,
            &sd,
            &directx::swap_chain,
            &directx::device,
            &feature_level,
            &directx::context
        );
        if (FAILED(hr))
            return false;
    }

    if (!directx::swap_chain || !directx::device || !directx::context)
        return false;

    ID3D11Texture2D* back_buffer = nullptr;
    hr = directx::swap_chain->GetBuffer(0, IID_PPV_ARGS(&back_buffer));
    if (FAILED(hr) || !back_buffer)
        return false;

    hr = directx::device->CreateRenderTargetView(back_buffer, nullptr, &directx::render_target_view);
    back_buffer->Release();
    back_buffer = nullptr;
    if (FAILED(hr) || !directx::render_target_view)
        return false;

    directx::context->OMSetRenderTargets(1, &directx::render_target_view, nullptr);

    D3D11_VIEWPORT viewport = {};
    viewport.Width = static_cast<float>(width);
    viewport.Height = static_cast<float>(height);
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    viewport.TopLeftX = 0.0f;
    viewport.TopLeftY = 0.0f;
    directx::context->RSSetViewports(1, &viewport);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;
    ImGui::StyleColorsDark();
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(directx::device, directx::context);

    return true;
}

void window::new_frame() {
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
}

void window::draw() {
    ImGui::EndFrame();
    ImGui::Render();
    static const float clearColor[4] = { 0.f, 0.f, 0.f, 0.f };
    directx::context->OMSetRenderTargets(1, &directx::render_target_view, nullptr);
    directx::context->ClearRenderTargetView(directx::render_target_view, clearColor);
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    directx::swap_chain->Present(0, 0); 
}

void window::cleanup() {
	if (window::hwnd) {
        ImGui_ImplDX11_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
		if (directx::render_target_view) { directx::render_target_view->Release(); directx::render_target_view = nullptr; }
		if (directx::swap_chain) { directx::swap_chain->Release(); directx::swap_chain = nullptr; }
		if (directx::context) { directx::context->Release(); directx::context = nullptr; }
		if (directx::device) { directx::device->Release(); directx::device = nullptr; }
		DestroyWindow(hwnd);
	}
}

bool window::create_window() {
    auto random_string = [](size_t length) -> const wchar_t* {
        const wchar_t charset[] = L"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
        const size_t charsetSize = wcslen(charset);
        wchar_t* randomString = new wchar_t[length + 1];
        static bool seeded = false;
        if (!seeded) {
            srand(static_cast<unsigned int>(time(nullptr)));
            seeded = true;
        }
        for (size_t i = 0; i < length; ++i) {
            randomString[i] = charset[rand() % charsetSize];
        }
        randomString[length] = L'\0';
        return randomString;
        };
    
    auto window_name = random_string(16);
    
    WNDCLASSEX wc = {};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW; 
    wc.lpfnWndProc = window::WndProc; 
    wc.hInstance = GetModuleHandle(nullptr);
    wc.lpszClassName = window_name;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW); 
    wc.hbrBackground = nullptr; 
    
    if (!RegisterClassEx(&wc))
        return false;
    
    window::width = GetSystemMetrics(SM_CXSCREEN);
    window::height = GetSystemMetrics(SM_CYSCREEN);
    
    window::hwnd = CreateWindowEx(
        WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW,
        window_name,
        random_string(16),
        WS_POPUP, 
        0, 0, window::width, window::height,
        nullptr, nullptr,
        wc.hInstance,
        nullptr
    );
    
    if (!window::hwnd) {
        UnregisterClass(window_name, wc.hInstance);
        return false;
    }

    MARGINS margin = { -1, -1, -1, -1 };
    HRESULT hr = DwmExtendFrameIntoClientArea(window::hwnd, &margin);
    if (FAILED(hr)) {
        UnregisterClass(window_name, wc.hInstance);
        DestroyWindow(window::hwnd);
        window::hwnd = nullptr;
        return false;
    }

    ShowWindow(window::hwnd, SW_SHOW);
    return UpdateWindow(window::hwnd);
}

bool overlay::scale() {
	static RECT old;

	HWND foreground_window = GetForegroundWindow();

	if (foreground_window == target) {
		SetWindowPos(window::hwnd, GetWindow(foreground_window, GW_HWNDPREV), 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		UpdateWindow(window::hwnd);
	}

	RECT rc = {};
	POINT xy = {};

	if (!GetClientRect(target, &rc))
		return false;

	if (!ClientToScreen(target, &xy))
		return false;

	rc.left = xy.x;
	rc.top = xy.y;

	if (rc.left != old.left || rc.right != old.right || rc.top != old.top || rc.bottom != old.bottom) {
		old = rc;

		width = rc.right;
		height = rc.bottom;

		SetWindowPos(window::hwnd, nullptr, xy.x, xy.y, static_cast<int>(width), static_cast<int>(height), SWP_NOREDRAW);
	}

	return true;
}

void overlay::click_through(bool click) {
    SetWindowLong(window::hwnd, GWL_EXSTYLE, click ? WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW : WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW);
}

bool overlay::initialize(const wchar_t* window_class, const wchar_t* window_name)
{
    if (!window_class)
        return false;

    overlay::target = FindWindowW(window_class, window_name);

    if (!overlay::target)
    {
        overlay::target = 0;
        return false;
    }

    if (!window::create_window())
        return false;


    if (!window::create_device())
        return false;

    return true;
}
