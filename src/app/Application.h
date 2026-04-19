#pragma once

#include "core/AuditTypes.h"

#include <filesystem>
#include <optional>
#include <vector>

namespace sec {

class Application {
public:
    int run(int argc, char** argv);

private:
    int runAudit(const std::vector<std::string>& args,
                 const std::optional<std::filesystem::path>& reportPath,
                 const std::optional<std::filesystem::path>& jsonPath);
    int runCleanup(const std::vector<std::string>& args,
                   const std::optional<std::filesystem::path>& reportPath,
                   const std::optional<std::filesystem::path>& jsonPath);
    int runFull(const std::vector<std::string>& args,
                const std::optional<std::filesystem::path>& reportPath,
                const std::optional<std::filesystem::path>& jsonPath);
    int runPractice(const std::vector<std::string>& args,
                    const std::optional<std::filesystem::path>& reportPath,
                    const std::optional<std::filesystem::path>& jsonPath);
    int runLearn(const std::optional<std::filesystem::path>& reportPath,
                 const std::optional<std::filesystem::path>& jsonPath,
                 bool studyOnly);
    void writeOutputs(const ExecutionBundle& bundle,
                      const std::optional<std::filesystem::path>& reportPath,
                      const std::optional<std::filesystem::path>& jsonPath) const;
    void printHelp() const;
};

}  // namespace sec
