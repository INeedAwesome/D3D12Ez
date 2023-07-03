#include <iostream>

#include "Support/WinInclude.h"
#include "Support/ComPointer.h"
#include "Support/Window.h"

#include "Debug/DXDebugLayer.h"
#include "D3D/DXContext.h"
#include "Debug/Timer.h"

#include <memory>
#include "Window/LauncherWindow.h"

int main(int argc, char* argv[])
{
	strt::LauncherWindow::Get().Init(450, 300);
	strt::LauncherSettings launcherSettings = strt::LauncherWindow::Get().Open();

	Timer init("DirectX & Window Init");

	std::cout << "Hello World!" << std::endl;
	DXDebugLayer::Get().Init();

	if (!DXContext::Get().Init())
		return 1;

	if (!DXWindow::Get().Init())
		return 2;

	if (launcherSettings.fullscreen)
		DXWindow::Get().SetFullscreen(true);

	init.StopAndPrintTime();

	bool running = true;
	while (!DXWindow::Get().ShouldClose())
	{
		DXWindow::Get().Update();
		if (DXWindow::Get().ShouldResize())
		{
			DXContext::Get().Flush(DXWindow::GetFrameCount());
			DXWindow::Get().Resize();
		}

		auto* commandList = DXContext::Get().InitCommandList();

		// a lot of setup
		// a draw


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