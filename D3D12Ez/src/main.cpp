#include <iostream>

#include "Support/WinInclude.h"
#include "Support/ComPointer.h"
#include "Support/Window.h"

#include "Debug/DXDebugLayer.h" 
#include "D3D/DXContext.h"
#include "Debug/Timer.h"

#ifdef EZ_DIST
int WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR, int)
#endif //EZ_DIST
#ifndef EZ_DIST
int main(int argc, char* argv[]) 
#endif //EZ_DIST
{  
	Timer init("DirectX & Window Init");
	  
	std::cout << "Hello World!" << std::endl; 
	DXDebugLayer::Get().Init(); 

	if (!DXContext::Get().Init())
		return 1;
	 
	if (!DXWindow::Get().Init())
		return 2;

	DXWindow::Get().SetFullscreen(true);

	init.StopAndPrintTime();

	while (!DXWindow::Get().ShouldClose())
	{
		DXWindow::Get().Update(); 
		if (DXWindow::Get().ShouldResize())
		{
			DXContext::Get().Flush(DXWindow::GetFrameCount());
			DXWindow::Get().Resize();
		}

		// begin drawing
		auto* commandList = DXContext::Get().InitCommandList(); 
		DXWindow::Get().BeginFrame(commandList);

		// draw


		// end draw
		DXWindow::Get().EndFrame(commandList);
		DXContext::Get().ExecuteCommandList();

		// presenting it to the screen 
		DXWindow::Get().Present();
	}

	//flushing
	DXContext::Get().Flush(DXWindow::GetFrameCount());

	DXWindow::Get().Shutdown();
	DXContext::Get().Shutdown();
	DXDebugLayer::Get().Shutdown();
	

	return 0;
}