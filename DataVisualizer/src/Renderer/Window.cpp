#include "pcHeader.h"
#include "Window.h"
#include "imgui.h"
#include "backends/imgui_impl_win32.h"
#include "backends/imgui_impl_vulkan.h"

bool window::init(const LPCWSTR title, uint32_t width, uint32_t height)
{
	width_ = width;
	height_ = height;

	window_ = create_window(title);
	if (!window_)
	{
		return false;
	}


	RECT rect;
	GetClientRect(window_, &rect);
	width_ = rect.right - rect.left;
	height_ = rect.bottom - rect.top;

	LOG_INFO("Initialized Window");
	return true;
}

void window::shutdown() const
{
	DestroyWindow(window_);
	LOG_INFO("Shutdown Window");
}

bool window::update() const
{
	MSG msg;
	PeekMessage(&msg, window_, 0, 0, PM_REMOVE);

	if (msg.message == WM_CLOSE) {
		if (msg.hwnd == window_)
		{
			DestroyWindow(window_);
			return false;
		}
	}

	TranslateMessage(&msg);
	DispatchMessage(&msg);
	RedrawWindow(window_, nullptr, nullptr, RDW_INTERNALPAINT);
	return true;
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK window::events(const HWND hwnd, const UINT msg, const WPARAM wparam, const LPARAM lparam)
{
	if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam))
		return true;


	switch (msg)
	{
	case WM_CLOSE:
		PostMessage(hwnd, msg, wparam, lparam);
		break;
	default:
		return DefWindowProc(hwnd, msg, wparam, lparam);
	}
	return 0;
}

HWND window::create_window(const LPCWSTR title)
{
	hinstance_ = GetModuleHandle(nullptr);

	WNDCLASSEX window_info{};
	window_info.cbSize = sizeof(WNDCLASSEX);
	window_info.style = CS_HREDRAW | CS_VREDRAW;
	window_info.lpfnWndProc = events;
	window_info.cbClsExtra = 0;
	window_info.cbWndExtra = 0;
	window_info.hInstance = hinstance_;
	window_info.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
	window_info.hCursor = LoadCursor(nullptr, IDC_ARROW);
	window_info.hbrBackground = static_cast<HBRUSH>(GetStockObject(WHITE_BRUSH));
	window_info.lpszMenuName = nullptr;
	window_info.lpszClassName = title;
	window_info.hIconSm = nullptr;

	if (const bool result = RegisterClassEx(&window_info); !result)
	{
		return nullptr;
	}

	RECT window_rect = { 0, 0, static_cast<LONG>(width_), static_cast<LONG>(height_) };

	AdjustWindowRect(&window_rect, WS_OVERLAPPEDWINDOW, FALSE);

	LOG_INFO("Created Windows Window");
	return CreateWindowEx(
		0,
		title,
		title,
		WS_OVERLAPPEDWINDOW | WS_VISIBLE,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		nullptr,
		nullptr,
		hinstance_,
		nullptr
	);
}
