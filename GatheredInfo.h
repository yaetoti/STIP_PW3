#pragma once

#include <string>
#include <vector>

struct GatheredInfo {
	std::wstring username;
	std::wstring computerName;
	std::wstring windowsDirectory;
	std::wstring systemDirectory;
	std::wstring volumeLabel;
	uint64_t memoryCapacity;
	int mouseButtonsCount;
	int screenWidth;

	explicit GatheredInfo(const std::wstring& path);

	std::vector<uint8_t> Serialize() const;
	std::wstring ToString() const;
};
