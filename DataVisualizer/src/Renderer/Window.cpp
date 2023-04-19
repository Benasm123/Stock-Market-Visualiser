#include "pcHeader.h"
#include "Window.h"

#include <functional>

#include "imgui.h"
#include "backends/imgui_impl_vulkan.h"

bool Window::Init(const LPCWSTR title, const uint32_t width, const uint32_t height)
{
	width_ = width;
	height_ = height;

	window_ = Window::CreateWinWindow(title, width, height);

	if (!window_) return false;

	RECT rect;
	GetClientRect(window_, &rect);
	width_ = rect.right - rect.left;
	height_ = rect.bottom - rect.top;

	LOG_INFO("Initialized Window");
	return true;
}

void Window::Shutdown() const
{
	DestroyWindow(window_);
	LOG_INFO("Shutdown Window");
}

bool Window::Update() const
{
	if (resized_)
	{
		LOG_INFO("CALLING ON RESIZE");
		for (const auto& function : onResizeFunctions_)
		{
			function();
		}
		resized_ = false;
	}

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

void Window::AddCallbackOnResize(const std::function<void()>& function)
{
	onResizeFunctions_.push_back(function);
}

extern IMGUI_IMPL_API LRESULT ImGuiImplWin32WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK Window::Events(const HWND hwnd, const UINT msg, const WPARAM wparam, const LPARAM lparam)
{
	ImGuiImplWin32WndProcHandler(hwnd, msg, wparam, lparam);

	switch (msg)
	{
	case WM_CLOSE:
		closed_ = true;
		PostMessage(hwnd, msg, wparam, lparam);
		break;
	case WM_SIZE:
		resized_ = true;
		break;
	default:
		return DefWindowProc(hwnd, msg, wparam, lparam);
	}
	return 0;
}

uint32_t Window::GetHeight() const
{
	RECT windowRect;
	GetClientRect(window_, &windowRect);
	return std::abs(windowRect.bottom - windowRect.top);
}

uint32_t Window::GetWidth() const
{
	RECT windowRect;
	GetClientRect(window_, &windowRect);
	return std::abs(windowRect.right - windowRect.left);
}

HWND Window::CreateWinWindow(const LPCWSTR title, const uint32_t width, const uint32_t height)
{
	hInstance_ = GetModuleHandle(nullptr);

	WNDCLASSEX windowInfo{};
	windowInfo.cbSize = sizeof(WNDCLASSEX);
	windowInfo.style = CS_HREDRAW | CS_VREDRAW;
	windowInfo.lpfnWndProc = Events;
	windowInfo.cbClsExtra = 0;
	windowInfo.cbWndExtra = 0;
	windowInfo.hInstance = hInstance_;
	windowInfo.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
	windowInfo.hCursor = LoadCursor(nullptr, IDC_ARROW);
	windowInfo.hbrBackground = static_cast<HBRUSH>(GetStockObject(WHITE_BRUSH));
	windowInfo.lpszMenuName = nullptr;
	windowInfo.lpszClassName = title;
	windowInfo.hIconSm = nullptr;

	if (const bool result = RegisterClassEx(&windowInfo); !result)
	{
		return nullptr;
	}

	RECT windowRect = { 0, 0, static_cast<LONG>(width_), static_cast<LONG>(height_) };

	AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

	LOG_INFO("Created Windows Window");
	return CreateWindowEx(
		0,
		title,
		title,
		WS_OVERLAPPEDWINDOW | WS_VISIBLE,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		width,
		height,
		nullptr,
		nullptr,
		hInstance_,
		nullptr
	);
}
