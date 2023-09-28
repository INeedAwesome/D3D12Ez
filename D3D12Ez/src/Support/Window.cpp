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
		swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
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

	// create rtv heap
	D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc{};
	descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	descHeapDesc.NumDescriptors = GetFrameCount();
	descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	descHeapDesc.NodeMask = 0;

	if (FAILED(DXContext::Get().GetDevice()->CreateDescriptorHeap(&descHeapDesc, IID_PPV_ARGS(&m_rtvDescHeap))))
		return false;

	auto firstHandle = m_rtvDescHeap->GetCPUDescriptorHandleForHeapStart();
	auto handleIncrement = DXContext::Get().GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	for (uint32_t i = 0; i < GetFrameCount(); i++)
	{
		m_rtvHandles[i] = firstHandle;
		m_rtvHandles[i].ptr += handleIncrement * i;
	}

	if (!GetBuffers())
		return false;


	return true;

}

void DXWindow::Shutdown()
{
	ReleaseBuffers();

	m_rtvDescHeap.Release();

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
	ReleaseBuffers();

	RECT clientRect;
	if (!GetClientRect(m_hwnd, &clientRect))
		return;

	m_width = clientRect.right - clientRect.left;
	m_height = clientRect.bottom - clientRect.top;

	m_swapChain->ResizeBuffers(GetFrameCount(), m_width, m_height, DXGI_FORMAT_UNKNOWN, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH | DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING);
	m_shouldResize = false;

	GetBuffers();
}

void DXWindow::SetFullscreen(bool enabled)
{

	HMONITOR monitor = MonitorFromWindow(m_hwnd, MONITOR_DEFAULTTONEAREST);
	MONITORINFO monitorInfo = { 0 };
	monitorInfo.cbSize = sizeof(monitorInfo);
	GetMonitorInfoW(monitor, &monitorInfo);
	

	DWORD style = WS_OVERLAPPEDWINDOW | WS_VISIBLE;
	DWORD exStyle = WS_EX_OVERLAPPEDWINDOW | WS_EX_APPWINDOW;

	if (enabled)
	{
		style = WS_POPUP | WS_VISIBLE;
		exStyle = WS_EX_APPWINDOW;
	}

	SetWindowLongW(m_hwnd, GWL_STYLE, style);
	SetWindowLongW(m_hwnd, GWL_EXSTYLE, exStyle);

	m_isFullscreen = enabled;

	if (enabled)
	{
		SetWindowPos(m_hwnd, nullptr,
			monitorInfo.rcMonitor.left,
			monitorInfo.rcMonitor.top,
			monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left,
			monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top, SWP_NOZORDER
		);
	}
	else 
	{
#pragma warning( push )
#pragma warning( disable : 4244 ) // disable "double to int, possible loss of data"
		m_width /= 1.5;
		m_height /= 1.5;
#pragma warning( pop ) 
		CenterWindow();
	}
}

void DXWindow::CenterWindow()
{
	if (!m_hwnd)
		return;
	if (IsFullscreen())
		return;
	
	HMONITOR monitor = MonitorFromWindow(m_hwnd, MONITOR_DEFAULTTONEAREST);
	MONITORINFO monitorInfo = { 0 };
	monitorInfo.cbSize = sizeof(monitorInfo);
	GetMonitorInfoW(monitor, &monitorInfo);

	int monitorWidth = monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left;
	int monitorHeight = monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top;

	int x = (monitorWidth / 2 - m_width / 2) + monitorInfo.rcMonitor.left;
	int y = (monitorHeight / 2 - m_height / 2) + monitorInfo.rcMonitor.top;

	MoveWindow(m_hwnd, x, y, m_width, m_height, FALSE);
}

void DXWindow::BeginFrame(ID3D12GraphicsCommandList7* cmdList)
{
	m_currentBufferIndex = m_swapChain->GetCurrentBackBufferIndex();

	D3D12_RESOURCE_BARRIER barrier;
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = m_buffers[m_currentBufferIndex];
	barrier.Transition.Subresource = 0;
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;

	cmdList->ResourceBarrier(1, &barrier);

	float clearColor[4] = { 0.2f, 0.3f, 0.5f, 1.0f};
	cmdList->ClearRenderTargetView(m_rtvHandles[m_currentBufferIndex], clearColor, 0, nullptr);

	cmdList->OMSetRenderTargets(1, &m_rtvHandles[m_currentBufferIndex], false, nullptr);
}

void DXWindow::EndFrame(ID3D12GraphicsCommandList7* cmdList)
{
	D3D12_RESOURCE_BARRIER barrier;
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = m_buffers[m_currentBufferIndex];
	barrier.Transition.Subresource = 0;
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;

	cmdList->ResourceBarrier(1, &barrier);
}

bool DXWindow::GetBuffers()
{
	for (uint32_t i = 0; i < GetFrameCount(); i++)
	{
		if (FAILED(m_swapChain->GetBuffer(i, IID_PPV_ARGS(&m_buffers[i]))))
			return false;

		D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
		rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		rtvDesc.Texture2D.MipSlice = 0;
		rtvDesc.Texture2D.PlaneSlice = 0;

		DXContext::Get().GetDevice()->CreateRenderTargetView(m_buffers[i], &rtvDesc, m_rtvHandles[i]);
	}
	return true;
}

void DXWindow::ReleaseBuffers()
{
	for (uint32_t i = 0; i < GetFrameCount(); i++)
	{
		m_buffers[i].Release();
	}
}

LRESULT DXWindow::OnWindowMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_KEYDOWN: {
			if (wParam == VK_F11)
				Get().SetFullscreen(!Get().IsFullscreen());
			if (wParam == VK_ESCAPE)
				Get().m_shouldClose = true;
			return 0;
		}
		case WM_CLOSE: {
			Get().m_shouldClose = true;
			return 0;
		}
		case WM_SIZE: {
			if (lParam && (HIWORD(lParam) != Get().m_height || LOWORD(lParam) != Get().m_width))
				Get().m_shouldResize = true;
			return 0;
		}
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}
