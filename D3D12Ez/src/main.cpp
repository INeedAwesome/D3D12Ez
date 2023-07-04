#include <iostream>

#include "Support/WinInclude.h"
#include "Support/ComPointer.h"
#include "Support/Window.h"

#include "Debug/DXDebugLayer.h" 
#include "D3D/DXContext.h"
#include "Debug/Timer.h"

#include "Window/LauncherWindow.h"

int main(int argc, char* argv[])
{
	bool running = true; 

	strt::LauncherSettings settingsFromLauncher = { true, 400, 300}; 
	
	strt::LauncherWindow launcher;  
	launcher.Init(450, 300);

	while (running)
	{ 
		if (!launcher.ShouldClose())
		{
			running = false; 
		}

		launcher.Update();
	}
	settingsFromLauncher = launcher.Shutdown();
	
	Timer init("DirectX & Window Init");
	 
	std::cout << "Hello World!" << std::endl; 
	DXDebugLayer::Get().Init(); 

	if (!DXContext::Get().Init())
		return 1;

	if (!DXWindow::Get().Init())
		return 2;

	if (settingsFromLauncher.fullscreen)
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

		auto* commandList = DXContext::Get().InitCommandList(); 

		// draw


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