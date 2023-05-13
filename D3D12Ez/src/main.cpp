#include <iostream>

#include "Support/WinInclude.h"
#include "Support/ComPointer.h"

int main ()
{
	std::cout << "Hello World!" << std::endl;

	MessageBeep(MB_ICONERROR);

	return 0;
}