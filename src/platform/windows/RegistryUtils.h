#pragma once

#include <windows.h>

#include <optional>
#include <string>

namespace sec {

std::optional<DWORD> readRegistryDword(HKEY rootKey, const std::wstring& subKey, const std::wstring& valueName);
std::optional<std::wstring> readRegistryString(HKEY rootKey, const std::wstring& subKey, const std::wstring& valueName);
bool registryKeyExists(HKEY rootKey, const std::wstring& subKey);

}  // namespace sec
