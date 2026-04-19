#pragma once

#include <cstdint>
#include <string>

namespace sec {

struct ProcessExecutionResult {
    bool success {false};
    std::uint32_t exitCode {0};
    std::string output;
    std::string errorMessage;
};

class ProcessRunner {
public:
    bool isWslAvailable() const;
    bool hasWslDistribution() const;
    ProcessExecutionResult runPowerShell(const std::string& script) const;
    ProcessExecutionResult runWslShell(const std::string& script) const;

private:
    ProcessExecutionResult runCommandLine(const std::wstring& commandLine) const;
};

}  // namespace sec
