#include <fstream>
#include <Registry.h>
#include <Signature.h>
#include <GatheredInfo.h>

#include "LoginForm.h"
#include "Constants.h"
#include "resource.h"

LoginResult::LoginResult(User* user, LoginStatus result)
    : user(user), result(result) { }

LRESULT CALLBACK RepeatProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_INITDIALOG:
        // Save input data
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, lParam);
        break;
    case WM_CLOSE:
        EndDialog(hwnd, false);
        break;
    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK && HIWORD(wParam) == BN_CLICKED) {
            // Get input and check matching
            const std::wstring* original = (const std::wstring*)GetWindowLongPtrW(hwnd, GWLP_USERDATA);
            HWND hPassword = GetDlgItem(hwnd, IDC_EDIT_REPEAT);
            int passwordLength = GetWindowTextLengthW(hPassword);
            if (passwordLength != original->length()) {
                MessageBoxW(hwnd, L"Passwords don't match!", L"Warning", MB_OK | MB_ICONERROR);
                break;
            }

            std::wstring password;
            password.resize(passwordLength);
            GetDlgItemTextW(hwnd, IDC_EDIT_REPEAT, &password[0], passwordLength + 1);
            if (password != *original) {
                MessageBoxW(hwnd, L"Passwords don't match!", L"Warning", MB_OK | MB_ICONERROR);
                break;
            }

            EndDialog(hwnd, true);
        }

        break;
    default:
        return FALSE;
    }

    return TRUE;
}

LRESULT CALLBACK LoginProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_INITDIALOG:
    {
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, lParam);
        HWND hHandshake = GetDlgItem(hwnd, IDC_HANDSHAKE);
        std::wstring text = L"Handshake: " + std::to_wstring(((const LoginInput*)lParam)->handshake);
        SetWindowTextW(hHandshake, text.c_str());
        break;
    }
    case WM_CLOSE:
        EndDialog(hwnd, (INT_PTR)new LoginResult(nullptr, LoginStatus::CANCEL));
        break;
    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK && HIWORD(wParam) == BN_CLICKED) {
            // Read user input
            HWND hLogin = GetDlgItem(hwnd, IDC_EDIT1);
            HWND hPassword = GetDlgItem(hwnd, IDC_EDIT2);
            HWND hHandshake = GetDlgItem(hwnd, IDC_EDIT3);
            HWND hRegistry = GetDlgItem(hwnd, ID_EDIT_REGISTRY);
            int loginLength = GetWindowTextLengthW(hLogin);
            int passwordLength = GetWindowTextLengthW(hPassword);
            int handshakeLength = GetWindowTextLengthW(hHandshake);
            int registryLength = GetWindowTextLengthW(hRegistry);
            if (loginLength == 0 || passwordLength == 0 || handshakeLength == 0 || registryLength == 0) {
                break;
            }

            std::wstring username;
            std::wstring password;
            std::wstring handshake;
            std::wstring registry;
            username.resize(loginLength);
            password.resize(passwordLength);
            handshake.resize(handshakeLength);
            registry.resize(registryLength);
            GetDlgItemTextW(hwnd, IDC_EDIT1, &username[0], loginLength + 1);
            GetDlgItemTextW(hwnd, IDC_EDIT2, &password[0], passwordLength + 1);
            GetDlgItemTextW(hwnd, IDC_EDIT3, &handshake[0], handshakeLength + 1);
            GetDlgItemTextW(hwnd, ID_EDIT_REGISTRY, &registry[0], registryLength + 1);
            // Rat check
            {
                std::vector<uint8_t> originalInfo;
                if (ERROR_SUCCESS != RegReadBinaryData(HKEY_CURRENT_USER, registry, L"GatheredInfo", originalInfo)) {
                    MessageBoxW(hwnd, L"Such registry key doesn't exist!", L"Warning", MB_OK | MB_ICONERROR);
                    break;
                }
                // Load license (signature + key)
                std::ifstream file(L"license", std::ios::binary);
                if (!file.is_open()) {
                    MessageBoxW(hwnd, L"No license file!", L"Warning", MB_OK | MB_ICONERROR);
                    break;
                }

                size_t sigLength = 0;
                file >> sigLength;
                std::vector<uint8_t> signature;
                signature.resize(sigLength);
                file.read((char*)signature.data(), sigLength);
                size_t keyLength = 0;
                file >> keyLength;
                std::vector<uint8_t> key;
                key.resize(keyLength);
                file.read((char*)key.data(), keyLength);
                if (!file.good()) {
                    MessageBoxW(hwnd, L"License file corrupted!", L"Warning", MB_OK | MB_ICONERROR);
                    break;
                }

                int status = VerifyDigitalSignature(originalInfo, signature, key);
                if (status == -1) {
                    MessageBoxW(hwnd, L"Internal error during verification. Check your license file integrity!", L"Error", MB_OK | MB_ICONERROR);
                    break;
                }

                if (status == FALSE) {
                    MessageBoxW(hwnd, L"You little rat, don't tell me you tried to bypass the license algorithm!", L"Go to hell!", MB_OK | MB_ICONERROR);
                    EndDialog(hwnd, (INT_PTR)new LoginResult(nullptr, LoginStatus::CANCEL));
                    break;
                }
                // Gather info
                std::wstring directory;
                DWORD dirLength = GetCurrentDirectoryW(0, nullptr);
                directory.resize(dirLength);
                GetCurrentDirectoryW(dirLength, &directory[0]);

                auto info = std::make_unique<GatheredInfo>(directory);
                std::vector<uint8_t> data = info->Serialize();
                // Compare info
                if (data.size() != originalInfo.size()) {
                    MessageBoxW(hwnd, L"You little rat, don't tell me you tried to bypass the license algorithm!", L"Go to hell!", MB_OK | MB_ICONERROR);
                    EndDialog(hwnd, (INT_PTR)new LoginResult(nullptr, LoginStatus::CANCEL));
                    break;
                }

                for (int i = 0; i < data.size(); ++i) {
                    if (data[i] != originalInfo[i]) {
                        MessageBoxW(hwnd, L"You little rat, don't tell me you tried to bypass the license algorithm!", L"Go to hell!", MB_OK | MB_ICONERROR);
                        EndDialog(hwnd, (INT_PTR)new LoginResult(nullptr, LoginStatus::CANCEL));
                        return TRUE;
                    }
                }
            }

            // If user doesn't exist - show warning
            LoginInput* input = (LoginInput*)GetWindowLongPtrW(hwnd, GWLP_USERDATA);
            User* user = nullptr;
            for (const std::unique_ptr<User>& currentUser : input->database.users) {
                if (username == currentUser->username) {
                    user = currentUser.get();
                    break;
                }
            }

            if (user == nullptr) {
                MessageBoxW(hwnd, L"User with such name doesn't exist!", L"Warning", MB_OK | MB_ICONERROR);
                break;
            }

            if (user->isBlocked) {
                MessageBoxW(hwnd, L"Account is blocked!", L"Warning", MB_OK | MB_ICONERROR);
                break;
            }

            if (!IsHandshakeValid(input->handshake, std::stoi(handshake))) {
                MessageBoxW(hwnd, L"Wrong handshake!", L"Warning", MB_OK | MB_ICONERROR);
                break;
            }

            // If user wasn't registered
            if (user->password.empty()) {
                // Validate if set restriction
                if (user->isRestrictionEnabled && !IsPasswordValid(password)) {
                    MessageBoxW(hwnd, L"Password must contain latin, cyrillic characters and numbers!", L"Warning", MB_OK | MB_ICONERROR);
                    break;
                }

                // Ask to repeat password
                bool match = DialogBoxParamW(GetModuleHandleW(nullptr), MAKEINTRESOURCE(IDD_DIALOG_REPEAT), hwnd, RepeatProc, (LPARAM)&password);
                if (!match) {
                    break;
                }

                // Match - change pass and return
                user->password = password;
                EndDialog(hwnd, (INT_PTR)new LoginResult(user, LoginStatus::UPDATE));
                break;
            }

            // User was registered. Check password. 3 times
            if (user->password != password) {
                MessageBoxW(hwnd, L"Wrong password!", L"Warning", MB_OK | MB_ICONERROR);
                --input->attempts;
                // No attempts left. Exit
                if (input->attempts <= 0) {
                    EndDialog(hwnd, (INT_PTR)new LoginResult(nullptr, LoginStatus::CANCEL));
                }

                break;
            }

            // Password match. Authorize user
            EndDialog(hwnd, (INT_PTR)new LoginResult(user, LoginStatus::LOGIN));
        }

        break;
    default:
        return FALSE;
    }

    return TRUE;
}
