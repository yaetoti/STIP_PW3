#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <memory>
#include <string>

#include "ConsoleLib/Console.h"
#include "GatheredInfo.h"

// TODO: Create signature for GatheredInfo
// TODO: Installation
// TODO: Dialog forms
// TODO: Add STIP_PW2 project as submodule. Rewrite ConsoleLib dependencies for it
// TODO: STIP_PW2: Add field registry key name. Implement key validation

LSTATUS RegWriteBinaryData(HKEY rootKey, const std::wstring& keyPath, const std::wstring& valueName, const std::vector<uint8_t>& data) {
    // TODO: Remove logging
    HKEY hKey;
    LSTATUS status = RegCreateKeyExW(rootKey, keyPath.c_str(), 0, nullptr, 0, KEY_SET_VALUE, nullptr, &hKey, nullptr);
    if (status == ERROR_SUCCESS) {
        status = RegSetValueExW(hKey, valueName.c_str(), 0, REG_BINARY, data.data(), (DWORD)data.size());
        if (status != ERROR_SUCCESS) {
            Console::GetInstance()->WPrintF(L"Failed to write data to the registry. Error code: %d.\n", status);
        }

        RegCloseKey(hKey);
    }
    else {
        Console::GetInstance()->WPrintF(L"Failed to open/create the registry key. Error code: ", status);
    }

    return status;
}

LSTATUS RegReadBinaryData(HKEY rootKey, const std::wstring& keyPath, const std::wstring& valueName, std::vector<uint8_t>& data) {
    HKEY hKey;
    LSTATUS status = RegOpenKeyExW(rootKey, keyPath.c_str(), 0, KEY_QUERY_VALUE, &hKey);
    if (status == ERROR_SUCCESS) {
        DWORD dataSize = 0;
        status = RegQueryValueExW(hKey, valueName.c_str(), nullptr, nullptr, nullptr, &dataSize);
        if (status == ERROR_SUCCESS) {
            data.resize(dataSize);
            status = RegQueryValueExW(hKey, valueName.c_str(), nullptr, nullptr, data.data(), &dataSize);
            if (status != ERROR_SUCCESS) {
                Console::GetInstance()->WPrintF(L"Failed to read data from the registry. Error code: %d.\n", status);
            }
        }
        else {
            Console::GetInstance()->WPrintF(L"Failed to query the size of data in the registry. Error code: %d.\n", status);
        }

        RegCloseKey(hKey);
    }
    else {
        Console::GetInstance()->WPrintF(L"Failed to open the registry key for reading. Error code: %d.\n", status);
    }

    return status;
}

int WINAPI wWinMain(HINSTANCE, HINSTANCE, LPWSTR, int) {
	auto info = std::make_unique<GatheredInfo>(L"C:\\MyProg");
	Console::GetInstance()->WPrintF(L"%s\n", info->ToString().c_str());
	std::vector<uint8_t> data = info->Serialize();

    RegWriteBinaryData(HKEY_CURRENT_USER, L"Kostin A.S.", L"GatheredInfo", data);

	std::vector<uint8_t> data1;
    RegReadBinaryData(HKEY_CURRENT_USER, L"Kostin A.S.", L"GatheredInfo", data1);
    auto result = std::make_unique<GatheredInfo>(std::move(data1));
    Console::GetInstance()->WPrintF(L"%s\n", result->ToString().c_str());

	Console::GetInstance()->Pause();
	return 0;
}
