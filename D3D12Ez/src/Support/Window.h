#pragma once

#include "Support/WinInclude.h"
#include "Support/ComPointer.h"

class DXWindow
{
public:
	DXWindow(const DXWindow&) = delete;
	DXWindow& operator=(const DXWindow&) = delete;

	static DXWindow& Get()
	{
		static DXWindow instance;
		return instance;
	}

	bool Init();
	void Shutdown();
	void Update();

	inline bool ShouldClose()
	{
		return m_shouldClose;
	}

private:
	DXWindow() = default;

	static LRESULT CALLBACK OnWindowMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

	ATOM m_windowClass = 0;
	HWND m_window;
	bool m_shouldClose = false;

	ComPointer<IDXGISwapChain4> m_swapChain;
};

