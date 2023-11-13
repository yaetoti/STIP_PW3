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

struct InstallerInput {
    std::wstring path;
    std::wstring registryKey;
    std::wstring name;
};

LRESULT CALLBACK InstallerProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_INITDIALOG:
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, lParam);
        SetDlgItemTextW(hwnd, ID_INSTALLER_TEXT_DIRECTORY, L"");
        break;
    case WM_CLOSE:
        EndDialog(hwnd, FALSE);
        break;
    case WM_COMMAND:
    {
        auto input = (InstallerInput*)GetWindowLongPtrW(hwnd, GWLP_USERDATA);

        if (LOWORD(wParam) == ID_INSTALLER_BUTTON_FOLDER && HIWORD(wParam) == BN_CLICKED) {
            std::wstring path = BrowseFolder();
            if (!path.empty()) {
                SetDlgItemTextW(hwnd, ID_INSTALLER_TEXT_DIRECTORY, path.c_str());
                input->path = std::move(path);
            }

            HWND hRegistry = GetDlgItem(hwnd, ID_INSTALLER_EDIT_REGISTRY);
            HWND hName = GetDlgItem(hwnd, ID_INSTALLER_EDIT_NAME);
            HWND hButton = GetDlgItem(hwnd, ID_INSTALLER_BUTTON_INSTALL);
            int registryLength = GetWindowTextLengthW(hRegistry);
            int nameLength = GetWindowTextLengthW(hName);
            if (registryLength == 0 || nameLength == 0 || input->path.empty()) {
                EnableWindow(hButton, FALSE);
            }
            else {
                EnableWindow(hButton, TRUE);
            }
        }
        else if ((LOWORD(wParam) == ID_INSTALLER_EDIT_REGISTRY || LOWORD(wParam) == ID_INSTALLER_EDIT_NAME)
            && HIWORD(wParam) == EN_CHANGE) {
            // Too lazy to put this code into separate functions but not so lazy to write a long-ass comment about that
            HWND hRegistry = GetDlgItem(hwnd, ID_INSTALLER_EDIT_REGISTRY);
            HWND hName = GetDlgItem(hwnd, ID_INSTALLER_EDIT_NAME);
            HWND hButton = GetDlgItem(hwnd, ID_INSTALLER_BUTTON_INSTALL);
            int registryLength = GetWindowTextLengthW(hRegistry);
            int nameLength = GetWindowTextLengthW(hName);
            if (registryLength == 0 || nameLength == 0 || input->path.empty()) {
                EnableWindow(hButton, FALSE);
            }
            else {
                EnableWindow(hButton, TRUE);
            }
        }
        else if (LOWORD(wParam) == ID_INSTALLER_BUTTON_INSTALL && HIWORD(wParam) == BN_CLICKED) {
            HWND hRegistry = GetDlgItem(hwnd, ID_INSTALLER_EDIT_REGISTRY);
            HWND hName = GetDlgItem(hwnd, ID_INSTALLER_EDIT_NAME);
            int registryLength = GetWindowTextLengthW(hRegistry);
            int nameLength = GetWindowTextLengthW(hName);
            input->registryKey.resize(registryLength);
            input->name.resize(registryLength);
            GetDlgItemTextW(hwnd, ID_INSTALLER_EDIT_REGISTRY, &input->registryKey[0], registryLength + 1);
            GetDlgItemTextW(hwnd, ID_INSTALLER_EDIT_NAME, &input->name[0], nameLength + 1);
            EndDialog(hwnd, TRUE);
        }

        break;
    }
    default:
        return FALSE;
    }

    return TRUE;
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int) {
    auto installerData = std::make_unique<InstallerInput>();
    if (!DialogBoxParamW(hInstance, MAKEINTRESOURCE(ID_DIALOG_INSTALLER), nullptr, InstallerProc, (LPARAM)installerData.get())) {
        return 0;
    }
    // Write gathered data to registry
	auto info = std::make_unique<GatheredInfo>(installerData->path);
	std::vector<uint8_t> data = info->Serialize();
    if (ERROR_SUCCESS != RegWriteBinaryData(HKEY_CURRENT_USER, installerData->registryKey, L"GatheredInfo", data)) {
        MessageBoxW(nullptr, L"Error during license generation!", L"Error", MB_OK | MB_ICONERROR);
        return 1;
    }
    // Generate license file
    std::unique_ptr<Signature> signature(CreateDigitalSignature(data));
    if (!signature) {
        MessageBoxW(nullptr, L"Error during license generation!", L"Error", MB_OK | MB_ICONERROR);
        return 1;
    }

    {
        std::ofstream file(installerData->path + L"\\license", std::ios::trunc | std::ios::binary);
        file << signature->signature.size();
        file.write((const char*)signature->signature.data(), signature->signature.size());
        file << signature->key.size();
        file.write((const char*)signature->key.data(), signature->key.size());
    }

    if (!InstallResource(installerData->path, installerData->name + L".exe", MAKEINTRESOURCEW(IDR_RT_APPLICATION1), L"RT_APPLICATION")) {
        MessageBoxW(nullptr, L"Error during program installation!", L"Error", MB_OK | MB_ICONERROR);
        return 1;
    }

    MessageBoxW(nullptr, L"Program was installed successfully!", L"Information", MB_OK | MB_ICONINFORMATION);
    return 0;
}
