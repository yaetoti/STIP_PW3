#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <wincrypt.h>
#include <memory>
#include <string>
#include <vector>

#include "ConsoleLib/Console.h"
#include "GatheredInfo.h"
#include "Registry.h"
#include "Signature.h"

// TODO: Sign verification algorithm
// TODO: Installation
// TODO: Dialog forms
// TODO: Add STIP_PW2 project as submodule. Rewrite ConsoleLib dependencies for it
// TODO: STIP_PW2: Add field registry key name. Implement key validation

int WINAPI wWinMain(HINSTANCE, HINSTANCE, LPWSTR, int) {
	auto info = std::make_unique<GatheredInfo>(L"C:\\MyProg");
	Console::GetInstance()->WPrintF(L"%s\n", info->ToString().c_str());
	std::vector<uint8_t> data = info->Serialize();
    RegWriteBinaryData(HKEY_CURRENT_USER, L"Kostin A.S.", L"GatheredInfo", data);

	std::vector<uint8_t> data1;
    RegReadBinaryData(HKEY_CURRENT_USER, L"Kostin A.S.", L"GatheredInfo", data1);
    auto result = std::make_unique<GatheredInfo>(std::move(data1));
    Console::GetInstance()->WPrintF(L"%s\n", result->ToString().c_str());

    std::unique_ptr<Signature> signature(CreateDigitalSignature(data));
    for (uint8_t byte : signature->key) {
        Console::GetInstance()->WPrintF(L"0x%02x\n", byte);
    }

	Console::GetInstance()->Pause();
	return 0;
}
