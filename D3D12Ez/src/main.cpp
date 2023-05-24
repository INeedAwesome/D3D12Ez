#include <iostream>

#include "Support/WinInclude.h"
#include "Support/ComPointer.h"

#include "Debug/DXDebugLayer.h"
#include "D3D/DXContext.h"

int main ()
{
	std::cout << "Hello World!" << std::endl;
	DXDebugLayer::Get().Init();

	if (!DXContext::Get().Init())
		return 1;

	while (true)
	{
		auto* commandList = DXContext::Get().InitCommandList();
		DXContext::Get().ExecuteCommandList();
	}


	DXContext::Get().Shutdown();
	DXDebugLayer::Get().Shutdown();
	return 0;
}