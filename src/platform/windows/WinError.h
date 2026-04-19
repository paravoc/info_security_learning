#pragma once

#include <windows.h>

#include <string>

namespace sec {

std::string formatWindowsErrorMessage(DWORD errorCode);

}  // namespace sec
