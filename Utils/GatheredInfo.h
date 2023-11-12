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
	explicit GatheredInfo(const std::vector<uint8_t>& data);
	explicit GatheredInfo(std::vector<uint8_t>&& data);

	std::vector<uint8_t> Serialize() const;
	void Deserialize(const std::vector<uint8_t>& data);
	void Deserialize(std::vector<uint8_t>&& data);
	std::wstring ToString() const;
};
