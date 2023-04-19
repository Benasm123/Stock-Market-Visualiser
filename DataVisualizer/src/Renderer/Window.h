#pragma once
#include <functional>
#include <pcHeader.h>

typedef std::vector<std::function<void(void)>> FunctionList;

class Window
{
public:
	Window() = default;
	~Window() = default;

	bool Init(LPCWSTR title, uint32_t width, uint32_t height);
	void Shutdown() const;
	[[nodiscard]] bool Update() const;

	static void AddCallbackOnResize(const std::function<void(void)>& function);

	inline bool static closed_ = false;
	inline bool static resized_ = false;

private:
	HWND CreateWinWindow(const LPCWSTR title, uint32_t width, uint32_t height);

	static LRESULT Events(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

public:
	[[nodiscard]] uint32_t GetHeight() const;
	[[nodiscard]] uint32_t GetWidth() const;
	[[nodiscard]] HINSTANCE GetHInstance() const { return hInstance_; }
	[[nodiscard]] HWND GetHwnd() const { return window_; }

private:
	inline static FunctionList onResizeFunctions_{};

	uint32_t width_{};
	uint32_t height_{};

	HINSTANCE hInstance_{};
	HWND window_{};
};

