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
private:
	DXWindow() = default;

public:
	bool Init();
	void Shutdown();
	void Update();
	void Present();
	void Resize();
	void SetFullscreen(bool enabled);
	void CenterWindow();

	void BeginFrame(ID3D12GraphicsCommandList7* cmdList);
	void EndFrame(ID3D12GraphicsCommandList7* cmdList);

	inline uint32_t GetWidth() const { return m_width; }
	inline uint32_t GetHeight() const { return m_height; }

	inline bool ShouldClose() {	return m_shouldClose; }
	inline bool ShouldResize() { return m_shouldResize; }
	inline bool IsFullscreen() { return m_isFullscreen; }
	static constexpr uint32_t m_frameCount = 2;
	static constexpr uint32_t GetFrameCount() { return m_frameCount; }

private:
	bool GetBuffers();
	void ReleaseBuffers();

	static LRESULT CALLBACK OnWindowMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

	ATOM m_windowClass = 0;
	HWND m_hwnd = nullptr;
	bool m_shouldClose = false;
	
	UINT m_width = 1280;
	UINT m_height = 720;
	bool m_shouldResize = false;

	bool m_isFullscreen = false;

	ComPointer<IDXGISwapChain4> m_swapChain = nullptr;
	ComPointer<ID3D12Resource2> m_buffers[m_frameCount];
	uint32_t m_currentBufferIndex = 0;
	
	ComPointer<ID3D12DescriptorHeap> m_rtvDescHeap;
	D3D12_CPU_DESCRIPTOR_HANDLE m_rtvHandles[m_frameCount];

};

