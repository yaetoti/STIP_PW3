#define _CRT_SECURE_NO_WARNINGS

#include <Windows.h>
#include <cstdlib>
#include <memory>

#include "resource.h"
#include "Console.h"
#include "Constants.h"
#include "Database.h"
#include "LoginForm.h"
#include "UserPanel.h"
#include "AdminPanel.h"

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow) {
    srand(GetTickCount());
    Database database(kDatabaseFile);
    User* user;
    {
        LoginInput loginParams = { database, rand() % 1000, kAttempts };
        std::unique_ptr<LoginResult> result((LoginResult*)DialogBoxParamW(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), nullptr, LoginProc, (LPARAM)&loginParams));
        if (result->result == LoginStatus::CANCEL) {
            return 0;
        }

        if (result->result == LoginStatus::UPDATE) {
            database.Save();
        }

        user = result->user;
    }

    // Show main form
    UserPanelInput panelInput = { database, user };
    if (user->username == kAdminUsername) {
        DialogBoxParamW(hInstance, MAKEINTRESOURCE(IDD_ADMIN_PANEL), nullptr, AdminPanelProc, (LPARAM)&panelInput);
    }
    else {
        DialogBoxParamW(hInstance, MAKEINTRESOURCE(IDD_USER_PANEL), nullptr, UserPanelProc, (LPARAM)&panelInput);
    }

	return 0;
}
