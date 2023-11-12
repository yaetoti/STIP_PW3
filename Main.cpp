#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <memory>
#include <vector>
#include <fstream>

#include "ConsoleLib/Console.h"
#include "GatheredInfo.h"
#include "Registry.h"
#include "Signature.h"

// TODO: Installation
// TODO: Dialog forms
// TODO: Add STIP_PW2 project as submodule. Rewrite ConsoleLib dependencies for it
// TODO: STIP_PW2: Add field registry key name. Implement key validation

int WINAPI wWinMain(HINSTANCE, HINSTANCE, LPWSTR, int) {
	auto info = std::make_unique<GatheredInfo>(L"C:\\MyProg");
	Console::GetInstance()->WPrintF(L"%s\n", info->ToString().c_str());
	std::vector<uint8_t> data = info->Serialize();
    RegWriteBinaryData(HKEY_CURRENT_USER, L"Kostin A.S.", L"GatheredInfo", data);

    /*
	std::vector<uint8_t> data1;
    RegReadBinaryData(HKEY_CURRENT_USER, L"Kostin A.S.", L"GatheredInfo", data1);
    auto result = std::make_unique<GatheredInfo>(std::move(data1));
    Console::GetInstance()->WPrintF(L"%s\n", result->ToString().c_str());
    */

    std::unique_ptr<Signature> signature(CreateDigitalSignature(data));
    if (!signature) {
        Console::GetInstance()->WPrintF(L"SIG_ERR\n");
        return 1;
    }

    int status = VerifyDigitalSignature(data, signature->signature, signature->key);
    Console::GetInstance()->WPrintF(L"Verification result: %d\n", status);

    data[0] = ~data[0];
    status = VerifyDigitalSignature(data, signature->signature, signature->key);
    Console::GetInstance()->WPrintF(L"Verification result: %d\n", status);

    /* {
        std::ofstream file(L"license", std::ios::trunc | std::ios::binary);
        file.write((const char*)signature->key.data(), signature->key.size());
    }*/

	Console::GetInstance()->Pause();
	return 0;
}
