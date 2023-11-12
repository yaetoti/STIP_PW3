#include <Windows.h>
#include <wincrypt.h>
#include "Signature.h"
#include "ScopedHandle.h"
#include "Console.h"

std::unique_ptr<Signature> CreateDigitalSignature(const std::vector<uint8_t>& data) {
    ScopedHandle<HCRYPTPROV> hProv(NULL, [](HCRYPTPROV value) { CryptReleaseContext(value, 0); });
    ScopedHandle<HCRYPTKEY> hKey(NULL, CryptDestroyKey);
    ScopedHandle<HCRYPTHASH> hHash(NULL, CryptDestroyHash);
    auto result = std::make_unique<Signature>();

    if (!CryptAcquireContextW(hProv.Address(), nullptr, MS_ENHANCED_PROV, PROV_RSA_FULL, 0)
        && !CryptAcquireContextW(hProv.Address(), nullptr, MS_ENHANCED_PROV, PROV_RSA_FULL, CRYPT_NEWKEYSET)) {
        Console::GetInstance()->WPrintF(L"Failed to acquire key container context. Error code: 0x%x\n", GetLastError());
        return nullptr;
    }
    // Create public key
    if (!CryptGetUserKey(hProv.Get(), AT_SIGNATURE, hKey.Address())
        && !CryptGenKey(hProv.Get(), AT_SIGNATURE, CRYPT_EXPORTABLE, hKey.Address())) {
        Console::GetInstance()->WPrintF(L"Failed to acquire user key. Error code: 0x%x\n", GetLastError());
        return nullptr;
    }
    // Export public key
    DWORD blobLength = 0;
    if (!CryptExportKey(hKey.Get(), NULL, PUBLICKEYBLOB, 0, nullptr, &blobLength)) {
        Console::GetInstance()->WPrintF(L"Failed to get blob length. Error code: 0x%x\n", GetLastError());
        return nullptr;
    }

    result->key.resize(blobLength, 0);
    if (!CryptExportKey(hKey.Get(), NULL, PUBLICKEYBLOB, 0, result->key.data(), &blobLength)) {
        Console::GetInstance()->WPrintF(L"Failed to get blob length. Error code: 0x%x\n", GetLastError());
        return nullptr;
    }

    // Hash data
    if (!CryptCreateHash(hProv.Get(), CALG_SHA1, 0, 0, hHash.Address())) {
        Console::GetInstance()->WPrintF(L"Failed to create hash object. Error code: 0x%x\n", GetLastError());
        return nullptr;
    }

    if (!CryptHashData(hHash.Get(), data.data(), (DWORD)data.size(), 0)) {
        Console::GetInstance()->WPrintF(L"Failed to hash data. Error code: 0x%x\n", GetLastError());
        return nullptr;
    }
    // Sign data hash
    DWORD sigLength = 0;
    if (!CryptSignHashW(hHash.Get(), AT_SIGNATURE, nullptr, 0, nullptr, &sigLength)) {
        Console::GetInstance()->WPrintF(L"Failed to get hash signature length. Error code: 0x%x\n", GetLastError());
        return nullptr;
    }

    result->signature.resize(sigLength, 0);
    if (!CryptSignHashW(hHash.Get(), AT_SIGNATURE, nullptr, 0, result->signature.data(), &sigLength)) {
        Console::GetInstance()->WPrintF(L"Failed to sign the hash. Error code: 0x%x\n", GetLastError());
        return nullptr;
    }

    return result;
}

int VerifyDigitalSignature(const std::vector<uint8_t>& data, const std::vector<uint8_t>& signature, const std::vector<uint8_t>& key) {
    ScopedHandle<HCRYPTPROV> hProv(NULL, [](HCRYPTPROV value) { CryptReleaseContext(value, 0); });
    ScopedHandle<HCRYPTKEY> hKey(NULL, CryptDestroyKey);
    ScopedHandle<HCRYPTHASH> hHash(NULL, CryptDestroyHash);
    auto result = std::make_unique<Signature>();

    if (!CryptAcquireContextW(hProv.Address(), nullptr, MS_ENHANCED_PROV, PROV_RSA_FULL, 0)
        && !CryptAcquireContextW(hProv.Address(), nullptr, MS_ENHANCED_PROV, PROV_RSA_FULL, CRYPT_NEWKEYSET)) {
        Console::GetInstance()->WPrintF(L"Failed to acquire key container context. Error code: 0x%x\n", GetLastError());
        return -1;
    }
    // Import public key
    if (!CryptImportKey(hProv.Get(), key.data(), (DWORD)key.size(), 0, 0, hKey.Address())) {
        Console::GetInstance()->WPrintF(L"Failed to import public key. Error code: 0x%x\n", GetLastError());
        return -1;
    }
    // Hash data
    if (!CryptCreateHash(hProv.Get(), CALG_SHA1, 0, 0, hHash.Address())) {
        Console::GetInstance()->WPrintF(L"Failed to create hash object. Error code: 0x%x\n", GetLastError());
        return -1;
    }

    if (!CryptHashData(hHash.Get(), data.data(), (DWORD)data.size(), 0)) {
        Console::GetInstance()->WPrintF(L"Failed to hash data. Error code: 0x%x\n", GetLastError());
        return -1;
    }
    // Verify signature
    if (!CryptVerifySignatureW(hHash.Get(), signature.data(), (DWORD)signature.size(), hKey.Get(), nullptr, 0)) {
        DWORD status = GetLastError();
        if (status != NTE_BAD_SIGNATURE) {
            Console::GetInstance()->WPrintF(L"Failed verify signature. Error code: 0x%x\n", status);
            return -1;
        }

        return FALSE;
    }

    return TRUE;
}
