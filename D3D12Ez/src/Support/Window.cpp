#include "Window.h"

bool DXWindow::Init()
{
	WNDCLASSEXW wcex{};
	{
		wcex.cbSize = sizeof(wcex);
		wcex.style = CS_OWNDC;
		wcex.lpfnWndProc = &DXWindow::OnWindowMessage;
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = 0;
		wcex.hInstance = GetModuleHandleW(nullptr);
		wcex.hIcon = LoadIconW(nullptr, IDI_APPLICATION);
		wcex.hCursor = LoadCursorW(nullptr, IDC_ARROW);
		wcex.hbrBackground = nullptr;
		wcex.lpszMenuName = nullptr;
		wcex.lpszClassName = L"D3D12ExWndCls";
		wcex.hIconSm = LoadIconW(nullptr, IDI_APPLICATION);
	}
	m_windowClass = RegisterClassExW(&wcex);
	if (m_windowClass == 0)
		return false;

	POINT pos{ 0,0 };
	GetCursorPos(&pos);
	HMONITOR monitor = MonitorFromPoint(pos, MONITOR_DEFAULTTOPRIMARY);
	MONITORINFO monitorInfo = { 0 };
	monitorInfo.cbSize = sizeof(monitorInfo);
	GetMonitorInfoW(monitor, &monitorInfo);

	m_width = monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left;
	m_height = monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top;
	
	m_hwnd = CreateWindowExW(
		WS_EX_OVERLAPPEDWINDOW | WS_EX_APPWINDOW, 
		(LPCWSTR)m_windowClass, 
		L"D3D12Ez", 
		WS_OVERLAPPEDWINDOW | WS_VISIBLE, 
		monitorInfo.rcWork.left, 
		monitorInfo.rcWork.top,
		m_width, m_height,
		nullptr, 
		nullptr, 
		wcex.hInstance, 
		nullptr
	);

	if (!m_hwnd)
		return false;

	DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
	DXGI_SWAP_CHAIN_FULLSCREEN_DESC swapChainFullscreenDesc{};
	ComPointer<IDXGISwapChain1> sc1;
	{
		swapChainDesc.Width = m_width;
		swapChainDesc.Height = m_height;
		swapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		swapChainDesc.Stereo = false;
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SampleDesc.Quality = 0; // MMAA
		swapChainDesc.BufferUsage = DXGI_USAGE_BACK_BUFFER | DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount = GetFrameCount();
		swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
		swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH | DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
	
		swapChainFullscreenDesc.RefreshRate;
		swapChainFullscreenDesc.ScanlineOrdering;
		swapChainFullscreenDesc.Scaling;
		swapChainFullscreenDesc.Windowed = true;
	}

	auto& factory = DXContext::Get().GetFactory();
	factory->CreateSwapChainForHwnd(DXContext::Get().GetCommandQueue(), m_hwnd, &swapChainDesc, &swapChainFullscreenDesc, nullptr, &sc1);
	if (!sc1.QueryInterface(m_swapChain))
		return false;

	return true;

}

void DXWindow::Shutdown()
{
	if (m_swapChain)
		m_swapChain.Release();

	if (m_hwnd)
		DestroyWindow(m_hwnd);

	if (m_windowClass)
		UnregisterClassW((LPCWSTR)m_windowClass, GetModuleHandleW(nullptr));

}

void DXWindow::Update()
{
	MSG msg;
	while (PeekMessageW(&msg, m_hwnd, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}
}

void DXWindow::Present()
{
	m_swapChain->Present(1, 0);
}

void DXWindow::Resize()
{
	RECT clientRect;
	if (!GetClientRect(m_hwnd, &clientRect))
		return;

	m_width = clientRect.right - clientRect.left;
	m_height = clientRect.bottom - clientRect.top;

	m_swapChain->ResizeBuffers(GetFrameCount(), m_width, m_height, DXGI_FORMAT_UNKNOWN, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH | DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING);
	m_shouldResize = false;

}

LRESULT DXWindow::OnWindowMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_CLOSE: {
			Get().m_shouldClose = true;
			break;
		}
		case WM_SIZE: {
			if (lParam && (HIWORD(lParam) != Get().m_height || LOWORD(lParam) != Get().m_width))
				Get().m_shouldResize = true;
			break;
		}
	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
}
