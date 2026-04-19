#include "platform/windows/WinError.h"

#include "platform/windows/Encoding.h"

namespace sec {

std::string formatWindowsErrorMessage(const DWORD errorCode) {
    LPWSTR rawMessage = nullptr;
    const auto flags = FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS;
    const auto length = FormatMessageW(
        flags,
        nullptr,
        errorCode,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        reinterpret_cast<LPWSTR>(&rawMessage),
        0,
        nullptr);

    if (length == 0 || rawMessage == nullptr) {
        return "Windows error " + std::to_string(errorCode);
    }

    std::wstring message(rawMessage, rawMessage + length);
    LocalFree(rawMessage);

    while (!message.empty() && (message.back() == L'\r' || message.back() == L'\n' || message.back() == L' ' || message.back() == L'.')) {
        message.pop_back();
    }

    return toUtf8(message);
}

}  // namespace sec

