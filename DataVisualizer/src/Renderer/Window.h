#pragma once
#include <pcHeader.h>

class window
{
public:
	window() = default;
	~window() = default;

	bool init(const LPCWSTR title, uint32_t width, uint32_t height);
	void shutdown() const;
	bool update() const;

private:
	HWND create_window(const LPCWSTR title);

	static LRESULT events(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

public:
	[[nodiscard]] uint32_t get_height() const { return height_; }
	[[nodiscard]] uint32_t get_width() const { return width_; }
	[[nodiscard]] HINSTANCE get_hinstance() const { return hinstance_; }
	[[nodiscard]] HWND get_hwnd() const { return window_; }

private:
	uint32_t width_{};
	uint32_t height_{};

	HINSTANCE hinstance_{};
	HWND window_{};
};

