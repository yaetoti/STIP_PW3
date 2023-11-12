#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <ShlObj.h>
#include <memory>
#include <vector>
#include <fstream>
#include <Console.h>
#include <GatheredInfo.h>
#include <Registry.h>
#include <Signature.h>
#include <ScopedHandle.h>

#include "resource.h"

// TODO: Dialog forms
// Button "Select Folder"
// Static Text "Path"

// Edit Box "Filename" + Static Text
// Edit Box "Registry Key" + Static Text

// Button "Install" (Disabled)

bool InstallResource(const std::wstring& folderPath, const std::wstring& fileName, LPCWSTR lpResourceName, LPCWSTR lpResourceType) {
    if (!CreateDirectoryW(folderPath.c_str(), nullptr) && GetLastError() != ERROR_ALREADY_EXISTS) {
        return false;
    }
    // Load resource
    HRSRC hResource = nullptr;
    HGLOBAL hLoadedResource = nullptr;
    const void* data = nullptr;
    if (!(hResource = FindResourceW(GetModuleHandleW(nullptr), lpResourceName, lpResourceType))
        || !(hLoadedResource = LoadResource(GetModuleHandleW(nullptr), hResource))
        || !(data = LockResource(hLoadedResource))) {
        return false;
    }
    // Write resource
    std::wstring fullPath = folderPath + L"\\" + fileName;
    ScopedHandle<HANDLE> hFile(CreateFileW(fullPath.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr),
        INVALID_HANDLE_VALUE, CloseHandle);
    if (hFile.Get() == INVALID_HANDLE_VALUE
        || !WriteFile(hFile.Get(), data, SizeofResource(GetModuleHandleW(nullptr), hResource), nullptr, nullptr)) {
        return false;
    }

    return true;
}

std::wstring BrowseFolder() {
    BROWSEINFOW bi = { nullptr };
    bi.lpszTitle = L"Select folder";
    ScopedHandle<LPITEMIDLIST> idList(SHBrowseForFolderW(&bi), nullptr, CoTaskMemFree);
    if (idList.Get() != nullptr) {
        wchar_t path[MAX_PATH];
        if (SHGetPathFromIDListW(idList.Get(), path)) {
            return path;
        }
    }

    return L"";
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int) {
    std::wstring path = BrowseFolder();
    std::wstring filename = L"Application";
    Console::GetInstance()->WPrintF(L"%s\n", path.c_str());
    if (path.empty()) {
        return 1;
    }

    /*
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
    */

    InstallResource(path, filename + L".exe", MAKEINTRESOURCEW(IDR_RT_APPLICATION1), L"RT_APPLICATION");

	Console::GetInstance()->Pause();
	return 0;
}
