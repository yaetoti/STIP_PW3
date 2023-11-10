#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <memory>

#include "ConsoleLib/Console.h"
#include "GatheredInfo.h"

// TODO: Create signature for GatheredInfo
// TODO: Write fetched data to register in binary mode

int WINAPI wWinMain(HINSTANCE, HINSTANCE, LPWSTR, int) {
	auto info = std::make_unique<GatheredInfo>(L"C:\\MyProg");
	Console::GetInstance()->WPrintF(L"%s\n", info->ToString().c_str());
	std::vector<uint8_t> data = info->Serialize();

	Console::GetInstance()->Pause();
	return 0;
}
