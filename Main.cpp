#include <Windows.h>

#include "ConsoleLib/Console.h"

int WINAPI wWinMain(HINSTANCE, HINSTANCE, LPWSTR, int) {
	Console::GetInstance()->WPrintF(L"Hello, Submodules!\n");
	Console::GetInstance()->Pause();

	return 0;
}
