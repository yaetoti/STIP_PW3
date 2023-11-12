#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <memory>
#include <vector>
#include <fstream>

#include "Console.h"
#include "GatheredInfo.h"
#include "Registry.h"
#include "Signature.h"

// TODO: Installation
// TODO: Dialog forms

int WINAPI wWinMain(HINSTANCE, HINSTANCE, LPWSTR, int) {
	auto info = std::make_unique<GatheredInfo>(L"C:\\MyProg");
	Console::GetInstance()->WPrintF(L"%s\n", info->ToString().c_str());
	std::vector<uint8_t> data = info->Serialize();
    RegWriteBinaryData(HKEY_CURRENT_USER, L"Kostin A.S.", L"GatheredInfo", data);

    std::unique_ptr<Signature> signature(CreateDigitalSignature(data));
    if (!signature) {
        Console::GetInstance()->WPrintF(L"SIG_ERR\n");
        return 1;
    }

    {
        std::ofstream file(L"license", std::ios::trunc | std::ios::binary);
        file << signature->signature.size();
        file.write((const char*)signature->signature.data(), signature->signature.size());
        file << signature->key.size();
        file.write((const char*)signature->key.data(), signature->key.size());
    }

	Console::GetInstance()->Pause();
	return 0;
}
