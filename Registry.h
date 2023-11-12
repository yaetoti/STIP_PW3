#pragma once

#include <Windows.h>
#include <string>
#include <vector>

LSTATUS RegWriteBinaryData(HKEY rootKey, const std::wstring& keyPath, const std::wstring& valueName, const std::vector<uint8_t>& data);
LSTATUS RegReadBinaryData(HKEY rootKey, const std::wstring& keyPath, const std::wstring& valueName, std::vector<uint8_t>& data);
