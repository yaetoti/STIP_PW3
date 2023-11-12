#include "Registry.h"
#include "ScopedHandle.h"
#include "Console.h"

LSTATUS RegWriteBinaryData(HKEY rootKey, const std::wstring& keyPath, const std::wstring& valueName, const std::vector<uint8_t>& data) {
    ScopedHandle<HKEY> hKey(nullptr, RegCloseKey);
    LSTATUS status = NULL;
    if ((status = RegCreateKeyExW(rootKey, keyPath.c_str(), 0, nullptr, 0, KEY_SET_VALUE, nullptr, hKey.Address(), nullptr)) != ERROR_SUCCESS) {
        Console::GetInstance()->WPrintF(L"Failed to open/create the registry key. Error code: ", status);
        return status;
    }

    if ((status = RegSetValueExW(hKey.Get(), valueName.c_str(), 0, REG_BINARY, data.data(), (DWORD)data.size())) != ERROR_SUCCESS) {
        Console::GetInstance()->WPrintF(L"Failed to write data to the registry. Error code: %d.\n", status);
        return status;
    }

    return status;
}

LSTATUS RegReadBinaryData(HKEY rootKey, const std::wstring& keyPath, const std::wstring& valueName, std::vector<uint8_t>& data) {
    ScopedHandle<HKEY> hKey(nullptr, RegCloseKey);
    LSTATUS status = NULL;
    if ((status = RegOpenKeyExW(rootKey, keyPath.c_str(), 0, KEY_QUERY_VALUE, hKey.Address())) != ERROR_SUCCESS) {
        Console::GetInstance()->WPrintF(L"Failed to open the registry key for reading. Error code: %d.\n", status);
        return status;
    }

    DWORD dataSize = 0;
    if ((status = RegQueryValueExW(hKey.Get(), valueName.c_str(), nullptr, nullptr, nullptr, &dataSize)) != ERROR_SUCCESS) {
        Console::GetInstance()->WPrintF(L"Failed to query the size of data in the registry. Error code: %d.\n", status);
        return status;
    }

    data.resize(dataSize);
    if ((status = RegQueryValueExW(hKey.Get(), valueName.c_str(), nullptr, nullptr, data.data(), &dataSize)) != ERROR_SUCCESS) {
        Console::GetInstance()->WPrintF(L"Failed to read data from the registry. Error code: %d.\n", status);
        return status;
    }

    return status;
}
