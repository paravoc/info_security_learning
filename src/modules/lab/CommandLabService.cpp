#include "modules/lab/CommandLabService.h"

#include "core/StringUtils.h"
#include "platform/windows/ProcessRunner.h"

#include <algorithm>
#include <map>
#include <sstream>
#include <string>
#include <utility>

namespace {

std::string summarizeOutput(const std::string& output) {
    constexpr std::size_t kMaxLines = 18;
    constexpr std::size_t kMaxCharacters = 2600;

    std::ostringstream stream;
    const auto lines = sec::splitLines(output);
    const auto linesToShow = std::min(lines.size(), kMaxLines);

    std::size_t written = 0;
    for (std::size_t index = 0; index < linesToShow; ++index) {
        const auto& line = lines[index];
        if (written + line.size() + 1 > kMaxCharacters) {
            stream << "... output truncated ...\n";
            return stream.str();
        }
        stream << line << '\n';
        written += line.size() + 1;
    }

    if (lines.size() > linesToShow) {
        stream << "... output truncated ...\n";
    }

    auto result = stream.str();
    if (result.empty()) {
        return "(no output)";
    }
    return result;
}

sec::LiveCommandResult makeInfoResult(std::string platform,
                                      std::string category,
                                      std::string title,
                                      std::string note) {
    sec::LiveCommandResult result;
    result.platform = std::move(platform);
    result.category = std::move(category);
    result.title = std::move(title);
    result.purpose = "Служебная информация о практическом режиме.";
    result.success = false;
    result.note = std::move(note);
    result.output = "(no output)";
    return result;
}

}  // namespace

namespace sec {

std::vector<LiveCommandResult> CommandLabService::run(const std::vector<CommandReference>& commands,
                                                      const bool includeLinuxLive) const {
    constexpr int kMaxPerCategory = 2;
    constexpr int kMaxResults = 14;

    std::vector<LiveCommandResult> results;
    std::map<std::pair<std::string, std::string>, int> categoryCounters;

    const ProcessRunner runner;
    const bool wslAvailable = includeLinuxLive && runner.hasWslDistribution();

    if (includeLinuxLive && !wslAvailable) {
        results.push_back(makeInfoResult(
            "Linux",
            "Практика",
            "WSL не готов",
            "Linux live-проверки пропущены: либо `wsl.exe` недоступен, либо в системе еще не установлен Linux-дистрибутив."));
    } else if (!includeLinuxLive) {
        results.push_back(makeInfoResult(
            "Linux",
            "Практика",
            "Linux live-проверки отключены",
            "Чтобы запускать Linux-команды прямо из приложения, включи опцию `Пробовать Linux через WSL`."));
    }

    for (const auto& command : commands) {
        if (!command.runnableInsideApp) {
            continue;
        }

        if (command.platform == "Linux" && !wslAvailable) {
            continue;
        }

        auto key = std::make_pair(command.platform, command.category);
        if (categoryCounters[key] >= kMaxPerCategory) {
            continue;
        }
        if (static_cast<int>(results.size()) >= kMaxResults) {
            break;
        }

        const auto execution = command.platform == "Linux"
                                   ? runner.runWslShell(command.command)
                                   : runner.runPowerShell(command.command);

        LiveCommandResult result;
        result.platform = command.platform;
        result.category = command.category;
        result.title = command.title;
        result.command = command.command;
        result.purpose = command.purpose;
        result.success = execution.success;
        result.output = summarizeOutput(execution.output);
        result.note = execution.success
                          ? (command.platform == "Linux" ? "Команда выполнена через WSL." : "Команда выполнена через локальный PowerShell.")
                          : (execution.errorMessage.empty() ? "Команда завершилась с ошибкой." : execution.errorMessage);

        results.push_back(std::move(result));
        ++categoryCounters[key];
    }

    return results;
}

}  // namespace sec
