#pragma once

#include "Support/WinInclude.h"
#include "Support/ComPointer.h"


class DXDebugLayer
{
public:
	DXDebugLayer(const DXDebugLayer&) = delete;
	DXDebugLayer& operator=(const DXDebugLayer&) = delete;

	static DXDebugLayer& Get()
	{
		static DXDebugLayer instance;
		return instance;
	}

	bool Init();
	void Shutdown();



private:
	DXDebugLayer() = default;
#ifdef EZ_DEBUG
	ComPointer<ID3D12Debug6> m_d3d12Debug;
	ComPointer<IDXGIDebug1> m_dxgiDebug;
#endif //EZ_DEBUG

};

