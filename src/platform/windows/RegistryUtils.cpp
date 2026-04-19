#include "platform/windows/RegistryUtils.h"

#include <vector>

namespace sec {

std::optional<DWORD> readRegistryDword(const HKEY rootKey, const std::wstring& subKey, const std::wstring& valueName) {
    DWORD value = 0;
    DWORD size = sizeof(value);
    const auto status = RegGetValueW(rootKey, subKey.c_str(), valueName.c_str(), RRF_RT_REG_DWORD, nullptr, &value, &size);
    if (status != ERROR_SUCCESS) {
        return std::nullopt;
    }

    return value;
}

std::optional<std::wstring> readRegistryString(const HKEY rootKey, const std::wstring& subKey, const std::wstring& valueName) {
    DWORD type = 0;
    DWORD size = 0;
    const auto status = RegGetValueW(rootKey, subKey.c_str(), valueName.c_str(), RRF_RT_REG_SZ | RRF_RT_REG_EXPAND_SZ, &type, nullptr, &size);
    if (status != ERROR_SUCCESS || size == 0) {
        return std::nullopt;
    }

    std::vector<wchar_t> buffer(size / sizeof(wchar_t), L'\0');
    if (RegGetValueW(rootKey, subKey.c_str(), valueName.c_str(), RRF_RT_REG_SZ | RRF_RT_REG_EXPAND_SZ, &type, buffer.data(), &size) != ERROR_SUCCESS) {
        return std::nullopt;
    }

    std::wstring value(buffer.data());
    return value;
}

bool registryKeyExists(const HKEY rootKey, const std::wstring& subKey) {
    HKEY handle = nullptr;
    const auto status = RegOpenKeyExW(rootKey, subKey.c_str(), 0, KEY_READ, &handle);
    if (status == ERROR_SUCCESS && handle != nullptr) {
        RegCloseKey(handle);
        return true;
    }

    return false;
}

}  // namespace sec

