#include <Windows.h>
#include <sstream>

#include "GatheredInfo.h"
#include "ByteStream.h"

GatheredInfo::GatheredInfo(const std::wstring& path) {
	// Username
	DWORD usernameLength = 0;
	GetUserNameW(nullptr, &usernameLength);
	this->username.resize(usernameLength - 1);
	GetUserNameW(&this->username[0], &usernameLength);

	// Computer name
	DWORD computerNameLength = 0;
	GetComputerNameW(nullptr, &computerNameLength);
	this->computerName.resize(computerNameLength - 1);
	GetComputerNameW(&this->computerName[0], &computerNameLength);

	// OS Windows folder
	std::wstring windowsDirectory;
	UINT windowsDirectoryLength = GetWindowsDirectoryW(nullptr, 0);
	this->windowsDirectory.resize(windowsDirectoryLength - 1);
	GetWindowsDirectoryW(&this->windowsDirectory[0], windowsDirectoryLength);

	// Folder with system files
	UINT systemDirectoryLength = GetSystemDirectoryW(nullptr, 0);
	this->systemDirectory.resize(systemDirectoryLength - 1);
	GetSystemDirectoryW(&this->systemDirectory[0], systemDirectoryLength);

	// Mouse button count
	this->mouseButtonsCount = GetSystemMetrics(SM_CMOUSEBUTTONS);

	// Screen width
	this->screenWidth = GetSystemMetrics(SM_CXSCREEN);

	// Memory capacity
	MEMORYSTATUSEX status = { 0 };
	status.dwLength = sizeof(MEMORYSTATUSEX);
	GlobalMemoryStatusEx(&status);
	this->memoryCapacity = status.ullTotalPhys;

	// Disk label (Of installed application)
	wchar_t volumePath[MAX_PATH + 1];
	GetVolumePathNameW(path.c_str(), volumePath, MAX_PATH + 1);
	this->volumeLabel.resize(MAX_PATH);
	GetVolumeInformationW(volumePath, &this->volumeLabel[0], MAX_PATH + 1, nullptr, nullptr, nullptr, nullptr, 0);
	this->volumeLabel.erase(std::find(this->volumeLabel.begin(), this->volumeLabel.end(), '\0'), this->volumeLabel.end());
}

GatheredInfo::GatheredInfo(const std::vector<uint8_t>& data) {
	Deserialize(data);
}

GatheredInfo::GatheredInfo(std::vector<uint8_t>&& data) {
	Deserialize(std::move(data));
}

std::vector<uint8_t> GatheredInfo::Serialize() const {
	ByteStream stream;
	stream << username
		<< computerName
		<< windowsDirectory
		<< systemDirectory
		<< volumeLabel
		<< memoryCapacity
		<< mouseButtonsCount
		<< screenWidth;
	return stream.release();
}

void GatheredInfo::Deserialize(const std::vector<uint8_t>& data) {
	ByteStream stream(data);
	stream >> username
		>> computerName
		>> windowsDirectory
		>> systemDirectory
		>> volumeLabel
		>> memoryCapacity
		>> mouseButtonsCount
		>> screenWidth;
}

void GatheredInfo::Deserialize(std::vector<uint8_t>&& data) {
	ByteStream stream(std::move(data));
	stream >> username
		>> computerName
		>> windowsDirectory
		>> systemDirectory
		>> volumeLabel
		>> memoryCapacity
		>> mouseButtonsCount
		>> screenWidth;
}

std::wstring GatheredInfo::ToString() const {
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
