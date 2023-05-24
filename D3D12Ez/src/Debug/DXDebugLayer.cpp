#include "DXDebugLayer.h"

bool DXDebugLayer::Init()
{
#ifdef EZ_DEBUG
	// Init D3D12 Debug Layer
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&m_d3d12Debug))))
	{
		m_d3d12Debug->EnableDebugLayer();

		// Init DXGI Debug Layer
		if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&m_dxgiDebug))))
		{
			m_dxgiDebug->EnableLeakTrackingForThread();
			return true;
		}
	}

#endif // EZ_DEBUG
	return false;
}

void DXDebugLayer::Shutdown()
{
#ifdef EZ_DEBUG
	if (m_dxgiDebug)
	{
		OutputDebugString(L"DXGI Reports Living Device Objects: \n");
		m_dxgiDebug->ReportLiveObjects( DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_FLAGS( DXGI_DEBUG_RLO_DETAIL | DXGI_DEBUG_RLO_IGNORE_INTERNAL));
	}

	m_dxgiDebug.Release();
	m_d3d12Debug.Release();
#endif // EZ_DEBUG
}
