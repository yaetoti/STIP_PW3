#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <string>
#include <memory>
#include <sstream>

#include "ConsoleLib/Console.h"

#pragma comment(lib, "Advapi32.lib")
#pragma comment(lib, "User32.lib")

// TODO: Write fetched data to register in binary mode

struct GatheredInfo {
	std::wstring username;
	std::wstring computerName;
	std::wstring windowsDirectory;
	std::wstring systemDirectory;
	std::wstring volumeLabel;
	uint64_t memoryCapacity;
	int mouseButtonsCount;
	int screenWidth;

	std::wstring ToString() const {
		std::wstringstream buf;
		buf << L"GatheredInfo { \""
			<< username << L"\", \""
			<< computerName << L"\", \""
			<< windowsDirectory << L"\", \""
			<< systemDirectory << L"\", \""
			<< volumeLabel << L"\", "
			<< memoryCapacity << L", "
			<< mouseButtonsCount << L", "
			<< screenWidth << L" }";
		return buf.str();
	}
};

std::unique_ptr<GatheredInfo> GatherComputerInfo(const std::wstring& path) {
	auto info = std::make_unique<GatheredInfo>();
	// Username
	DWORD usernameLength = 0;
	GetUserNameW(nullptr, &usernameLength);
	info->username.resize(usernameLength - 1);
	GetUserNameW(&info->username[0], &usernameLength);

	// Computer name
	DWORD computerNameLength = 0;
	GetComputerNameW(nullptr, &computerNameLength);
	info->computerName.resize(computerNameLength - 1);
	GetComputerNameW(&info->computerName[0], &computerNameLength);

	// OS Windows folder
	std::wstring windowsDirectory;
	UINT windowsDirectoryLength = GetWindowsDirectoryW(nullptr, 0);
	info->windowsDirectory.resize(windowsDirectoryLength - 1);
	GetWindowsDirectoryW(&info->windowsDirectory[0], windowsDirectoryLength);

	// Folder with system files
	UINT systemDirectoryLength = GetSystemDirectoryW(nullptr, 0);
	info->systemDirectory.resize(systemDirectoryLength - 1);
	GetSystemDirectoryW(&info->systemDirectory[0], systemDirectoryLength);

	// Mouse button count
	info->mouseButtonsCount = GetSystemMetrics(SM_CMOUSEBUTTONS);

	// Screen width
	info->screenWidth = GetSystemMetrics(SM_CXSCREEN);

	// Memory capacity
	MEMORYSTATUSEX status = { 0 };
	status.dwLength = sizeof(MEMORYSTATUSEX);
	GlobalMemoryStatusEx(&status);
	info->memoryCapacity = status.ullTotalPhys;

	// Disk label (Of installed application)
	wchar_t volumePath[MAX_PATH + 1];
	GetVolumePathNameW(path.c_str(), volumePath, MAX_PATH + 1);
	info->volumeLabel.resize(MAX_PATH);
	GetVolumeInformationW(volumePath, &info->volumeLabel[0], MAX_PATH + 1, nullptr, nullptr, nullptr, nullptr, 0);
	info->volumeLabel.erase(std::find(info->volumeLabel.begin(), info->volumeLabel.end(), '\0'), info->volumeLabel.end());

	return std::move(info);
}

int WINAPI wWinMain(HINSTANCE, HINSTANCE, LPWSTR, int) {
	// Disk label (Of installed application)
	std::unique_ptr<GatheredInfo> info = GatherComputerInfo(L"C:\\MyProg");
	Console::GetInstance()->WPrintF(L"%s\n", info->ToString().c_str());

	Console::GetInstance()->Pause();
	return 0;
}
