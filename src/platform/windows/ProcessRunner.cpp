#include "platform/windows/ProcessRunner.h"

#include "core/StringUtils.h"
#include "platform/windows/Encoding.h"
#include "platform/windows/WinError.h"

#include <Windows.h>

#include <array>
#include <cstdint>
#include <vector>

namespace {

std::string encodeBase64(const std::vector<std::uint8_t>& data) {
    static constexpr char kAlphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    std::string encoded;
    encoded.reserve(((data.size() + 2) / 3) * 4);

    for (std::size_t index = 0; index < data.size(); index += 3) {
        const auto first = data[index];
        const auto second = index + 1 < data.size() ? data[index + 1] : 0;
        const auto third = index + 2 < data.size() ? data[index + 2] : 0;
        const auto chunk = (static_cast<std::uint32_t>(first) << 16U) |
                           (static_cast<std::uint32_t>(second) << 8U) |
                           static_cast<std::uint32_t>(third);

        encoded.push_back(kAlphabet[(chunk >> 18U) & 0x3FU]);
        encoded.push_back(kAlphabet[(chunk >> 12U) & 0x3FU]);
        encoded.push_back(index + 1 < data.size() ? kAlphabet[(chunk >> 6U) & 0x3FU] : '=');
        encoded.push_back(index + 2 < data.size() ? kAlphabet[chunk & 0x3FU] : '=');
    }

    return encoded;
}

std::wstring buildPowerShellCommandLine(const std::string& script) {
    const std::string wrappedScript =
        "$ProgressPreference='SilentlyContinue'; "
        "$ErrorActionPreference='Stop'; "
        "[Console]::OutputEncoding=[System.Text.Encoding]::UTF8; "
        "$OutputEncoding=[System.Text.Encoding]::UTF8; " + script;

    const auto wideScript = sec::toWide(wrappedScript);
    const auto* bytes = reinterpret_cast<const std::uint8_t*>(wideScript.data());
    const std::vector<std::uint8_t> scriptBytes(bytes, bytes + (wideScript.size() * sizeof(wchar_t)));
    const auto encoded = encodeBase64(scriptBytes);

    return L"powershell.exe -NoProfile -ExecutionPolicy Bypass -EncodedCommand " + sec::toWide(encoded);
}

std::wstring quoteWindowsArgument(const std::wstring& value) {
    if (value.find_first_of(L" \t\"") == std::wstring::npos) {
        return value;
    }

    std::wstring result = L"\"";
    std::size_t backslashCount = 0;
    for (const auto character : value) {
        if (character == L'\\') {
            ++backslashCount;
            continue;
        }
        if (character == L'"') {
            result.append(backslashCount * 2 + 1, L'\\');
            result.push_back(L'"');
            backslashCount = 0;
            continue;
        }
        if (backslashCount > 0) {
            result.append(backslashCount, L'\\');
            backslashCount = 0;
        }
        result.push_back(character);
    }

    if (backslashCount > 0) {
        result.append(backslashCount * 2, L'\\');
    }
    result.push_back(L'"');
    return result;
}

std::wstring buildWslCommandLine(const std::string& script) {
    const std::string wrappedScript = "export LANG=C.UTF-8 LC_ALL=C.UTF-8; " + script;
    return L"wsl.exe -e sh -lc " + quoteWindowsArgument(sec::toWide(wrappedScript));
}

}  // namespace

namespace sec {

bool ProcessRunner::isWslAvailable() const {
    wchar_t buffer[MAX_PATH] {};
    return SearchPathW(nullptr, L"wsl.exe", nullptr, MAX_PATH, buffer, nullptr) > 0;
}

bool ProcessRunner::hasWslDistribution() const {
    if (!isWslAvailable()) {
        return false;
    }

    const auto result = runCommandLine(L"wsl.exe --list --quiet");
    return result.success && !trim(result.output).empty();
}

ProcessExecutionResult ProcessRunner::runPowerShell(const std::string& script) const {
    return runCommandLine(buildPowerShellCommandLine(script));
}

ProcessExecutionResult ProcessRunner::runWslShell(const std::string& script) const {
    if (!isWslAvailable()) {
        return {false, 0, {}, "WSL is not available on this Windows host."};
    }
    if (!hasWslDistribution()) {
        return {
            false,
            0,
            {},
            "WSL is installed but no Linux distribution is configured. Use `wsl.exe --list --online` and `wsl.exe --install <Distro>` first.",
        };
    }

    return runCommandLine(buildWslCommandLine(script));
}

ProcessExecutionResult ProcessRunner::runCommandLine(const std::wstring& commandLine) const {
    SECURITY_ATTRIBUTES attributes {};
    attributes.nLength = sizeof(attributes);
    attributes.bInheritHandle = TRUE;

    HANDLE readPipe = nullptr;
    HANDLE writePipe = nullptr;
    if (!CreatePipe(&readPipe, &writePipe, &attributes, 0)) {
        return {false, 0, {}, "CreatePipe failed: " + formatWindowsErrorMessage(GetLastError())};
    }

    if (!SetHandleInformation(readPipe, HANDLE_FLAG_INHERIT, 0)) {
        CloseHandle(readPipe);
        CloseHandle(writePipe);
        return {false, 0, {}, "SetHandleInformation failed: " + formatWindowsErrorMessage(GetLastError())};
    }

    STARTUPINFOW startupInfo {};
    startupInfo.cb = sizeof(startupInfo);
    startupInfo.dwFlags = STARTF_USESTDHANDLES;
    startupInfo.hStdOutput = writePipe;
    startupInfo.hStdError = writePipe;
    startupInfo.hStdInput = GetStdHandle(STD_INPUT_HANDLE);

    PROCESS_INFORMATION processInfo {};
    std::vector<wchar_t> commandBuffer(commandLine.begin(), commandLine.end());
    commandBuffer.push_back(L'\0');

    const auto created = CreateProcessW(nullptr,
                                        commandBuffer.data(),
                                        nullptr,
                                        nullptr,
                                        TRUE,
                                        CREATE_NO_WINDOW,
                                        nullptr,
                                        nullptr,
                                        &startupInfo,
                                        &processInfo);

    CloseHandle(writePipe);

    if (!created) {
        CloseHandle(readPipe);
        return {false, 0, {}, "CreateProcessW failed: " + formatWindowsErrorMessage(GetLastError())};
    }

    std::string output;
    std::array<char, 4096> buffer {};
    DWORD bytesRead = 0;
    while (ReadFile(readPipe, buffer.data(), static_cast<DWORD>(buffer.size()), &bytesRead, nullptr) && bytesRead > 0) {
        output.append(buffer.data(), buffer.data() + bytesRead);
    }

    CloseHandle(readPipe);

    WaitForSingleObject(processInfo.hProcess, 120000);

    DWORD exitCode = 0;
    GetExitCodeProcess(processInfo.hProcess, &exitCode);

    CloseHandle(processInfo.hThread);
    CloseHandle(processInfo.hProcess);

    ProcessExecutionResult result;
    result.success = exitCode == 0;
    result.exitCode = exitCode;
    result.output = output;
    if (!result.success) {
        result.errorMessage = output.empty() ? "PowerShell command failed." : output;
    }
    return result;
}

}  // namespace sec
