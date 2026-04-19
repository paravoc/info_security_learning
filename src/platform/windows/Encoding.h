#pragma once

#include <string>

namespace sec {

std::string toUtf8(const std::wstring& value);
std::wstring toWide(const std::string& value);

}  // namespace sec

