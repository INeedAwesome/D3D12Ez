#pragma once

#include "Support/WinInclude.h"
#include "Support/ComPointer.h"
#include "D3D/DXContext.h"

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
	void Present();
	void Resize();
	void SetFullscreen(bool enabled);
	void CenterWindow();

	inline bool ShouldClose() {	return m_shouldClose; }
	inline bool ShouldResize() { return m_shouldResize; }
	inline bool IsFullscreen() { return m_isFullscreen; }
	static constexpr size_t GetFrameCount() { return 2; }

private:
	DXWindow() = default;

	static LRESULT CALLBACK OnWindowMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

	ATOM m_windowClass = 0;
	HWND m_hwnd;
	bool m_shouldClose = false;
	
	UINT m_width = 1280;
	UINT m_height = 720;
	bool m_shouldResize = false;

	bool m_isFullscreen = false;

	ComPointer<IDXGISwapChain4> m_swapChain;
};

