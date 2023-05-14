#include <iostream>

#include "Support/WinInclude.h"
#include "Support/ComPointer.h"

#include "Debug/DXDebugLayer.h"

int main ()
{
	std::cout << "Hello World!" << std::endl;
	DXDebugLayer::Get().Init();



	DXDebugLayer::Get().Shutdown();
	return 0;
}