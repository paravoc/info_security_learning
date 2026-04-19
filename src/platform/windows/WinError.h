#pragma once

#include <Windows.h>

#include <string>

namespace sec {

std::string formatWindowsErrorMessage(DWORD errorCode);

}  // namespace sec

