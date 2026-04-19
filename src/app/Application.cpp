#include "app/Application.h"

#include "app/WorkbenchService.h"
#include "core/StringUtils.h"
#include "history/HistoryStore.h"
#include "report/JsonWriter.h"
#include "report/ReportWriter.h"

#include <exception>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

namespace {

void printSectionHeader(const std::string& title) {
    std::cout << "\n=== " << title << " ===\n\n";
}

void printAuditItems(const std::vector<sec::AuditItem>& items) {
    if (items.empty()) {
        return;
    }

    printSectionHeader("Находки аудита");
    for (const auto& item : items) {
        std::cout << '[' << sec::toDisplayString(item.status) << "] " << item.category << " :: " << item.title << '\n';
        for (const auto& detail : item.details) {
            std::cout << "  - " << detail << '\n';
        }
        if (!item.recommendation.empty()) {
            std::cout << "  Рекомендация: " << item.recommendation << '\n';
        }
        std::cout << '\n';
    }
}

void printCleanupSummary(const sec::CleanupSummary& summary) {
    printSectionHeader("Очистка и кэш");
    std::cout << "Режим: " << (summary.dryRun ? "dry-run" : "применение") << "\n\n";
    for (const auto& target : summary.targets) {
        const auto state = target.skipped ? "ПРОПУЩЕНО" : (target.success ? "OK" : "ВНИМАНИЕ");
        std::cout << '[' << state << "] " << target.target << '\n';
        std::cout << "  - Файлы: " << target.filesAffected << '\n';
        std::cout << "  - Данные: " << sec::humanReadableBytes(target.bytesAffected) << '\n';
        std::cout << "  - Примечание: " << target.message << "\n\n";
    }
}

void printCommands(const std::vector<sec::CommandReference>& commands) {
    if (commands.empty()) {
        return;
    }

    printSectionHeader("Команды и шпаргалки");
    std::string currentPlatform;
    std::string currentCategory;

    for (const auto& command : commands) {
        if (command.platform != currentPlatform || command.category != currentCategory) {
            currentPlatform = command.platform;
            currentCategory = command.category;
            std::cout << "--- [" << currentPlatform << "] " << currentCategory << " ---\n\n";
        }

        std::cout << command.title << '\n';
        if (!command.relatedTool.empty()) {
            std::cout << "  Связано с GUI: " << command.relatedTool << '\n';
        }
        if (!command.scenario.empty()) {
            std::cout << "  Когда использовать: " << command.scenario << '\n';
        }
        std::cout << "  Команда      : " << command.command << '\n';
        std::cout << "  Назначение   : " << command.purpose << '\n';
        std::cout << "  Осторожно    : " << command.caution << '\n';
        if (!command.exampleOutput.empty()) {
            std::cout << "  Что получится:\n";
            std::cout << "    ------------------------------\n";
            for (const auto& line : sec::splitLines(command.exampleOutput)) {
                std::cout << "    " << line << '\n';
            }
            std::cout << "    ------------------------------\n";
        }
        if (!command.interpretation.empty()) {
            std::cout << "  Как читать   : " << command.interpretation << '\n';
        }
        if (!command.checks.empty()) {
            std::cout << "  На что смотреть:\n";
            for (const auto& value : command.checks) {
                std::cout << "    - " << value << '\n';
            }
        }
        if (!command.nextCommands.empty()) {
            std::cout << "  Что проверить потом:\n";
            for (const auto& value : command.nextCommands) {
                std::cout << "    - " << value << '\n';
            }
        }
        std::cout << "  Практика в приложении: " << (command.runnableInsideApp ? "да" : "нет") << '\n';
        std::cout << '\n';
    }
}

void printTools(const std::vector<sec::SystemToolReference>& tools) {
    if (tools.empty()) {
        return;
    }

    printSectionHeader("Системные инструменты и оснастки");
    std::string currentPlatform;
    std::string currentCategory;

    for (const auto& tool : tools) {
        if (tool.platform != currentPlatform || tool.category != currentCategory) {
            currentPlatform = tool.platform;
            currentCategory = tool.category;
            std::cout << "--- [" << currentPlatform << "] " << currentCategory << " ---\n\n";
        }

        std::cout << tool.title << '\n';
        std::cout << "  Где открыть  : " << tool.openTarget << '\n';
        std::cout << "  Для чего     : " << tool.purpose << '\n';
        std::cout << "  Когда идти   :\n";
        for (const auto& value : tool.useCases) {
            std::cout << "    - " << value << '\n';
        }
        std::cout << "  Что смотреть :\n";
        for (const auto& value : tool.whatToCheck) {
            std::cout << "    - " << value << '\n';
        }
        std::cout << "  Команды рядом:\n";
        for (const auto& value : tool.relatedCommands) {
            std::cout << "    - " << value << '\n';
        }
        std::cout << '\n';
    }
}

void printArtifactLocations(const std::vector<sec::ArtifactLocation>& locations) {
    if (locations.empty()) {
        return;
    }

    printSectionHeader("Где что лежит");
    std::string currentPlatform;
    std::string currentCategory;

    for (const auto& location : locations) {
        if (location.platform != currentPlatform || location.category != currentCategory) {
            currentPlatform = location.platform;
            currentCategory = location.category;
            std::cout << "--- [" << currentPlatform << "] " << currentCategory << " ---\n\n";
        }

        std::cout << location.artifact << '\n';
        std::cout << "  Где искать   :\n";
        for (const auto& path : location.locations) {
            std::cout << "    - " << path << '\n';
        }
        std::cout << "  Команда      : " << location.inspectionCommand << '\n';
        std::cout << "  Зачем смотреть: " << location.purpose << "\n\n";
    }
}

void printLearningSections(const std::vector<sec::LearningSection>& sections) {
    if (sections.empty()) {
        return;
    }

    printSectionHeader("План обучения");
    for (const auto& section : sections) {
        std::cout << section.audience << " :: " << section.title << '\n';
        std::cout << "  Что изучить:\n";
        for (const auto& topic : section.topics) {
            std::cout << "    - " << topic << '\n';
        }
        std::cout << "  Практика:\n";
        for (const auto& lab : section.labs) {
            std::cout << "    - " << lab << '\n';
        }
        std::cout << "  Цели:\n";
        for (const auto& milestone : section.milestones) {
            std::cout << "    - " << milestone << '\n';
        }
        std::cout << '\n';
    }
}

void printPlaybooks(const std::vector<sec::PracticePlaybook>& playbooks) {
    if (playbooks.empty()) {
        return;
    }

    printSectionHeader("Практические сценарии");
    std::string currentPlatform;
    std::string currentCategory;

    for (const auto& playbook : playbooks) {
        if (playbook.platform != currentPlatform || playbook.category != currentCategory) {
            currentPlatform = playbook.platform;
            currentCategory = playbook.category;
            std::cout << "--- [" << currentPlatform << "] " << currentCategory << " ---\n\n";
        }

        std::cout << playbook.title << '\n';
        std::cout << "  Симптом      : " << playbook.symptom << '\n';
        std::cout << "  Цель         : " << playbook.goal << '\n';
        std::cout << "  Что сделать  :\n";
        for (const auto& step : playbook.steps) {
            std::cout << "    - " << step << '\n';
        }
        std::cout << "  Команды      :\n";
        for (const auto& value : playbook.commands) {
            std::cout << "    - " << value << '\n';
        }
        std::cout << "  Что должно насторожить:\n";
        for (const auto& value : playbook.expectedSignals) {
            std::cout << "    - " << value << '\n';
        }
        if (!playbook.relatedTools.empty()) {
            std::cout << "  Куда еще зайти:\n";
            for (const auto& value : playbook.relatedTools) {
                std::cout << "    - " << value << '\n';
            }
        }
        std::cout << '\n';
    }
}

void printLiveResults(const std::vector<sec::LiveCommandResult>& results) {
    if (results.empty()) {
        return;
    }

    printSectionHeader("Живые проверки");
    std::string currentPlatform;
    std::string currentCategory;

    for (const auto& result : results) {
        if (result.platform != currentPlatform || result.category != currentCategory) {
            currentPlatform = result.platform;
            currentCategory = result.category;
            std::cout << "--- [" << currentPlatform << "] " << currentCategory << " ---\n\n";
        }

        std::cout << '[' << (result.success ? "OK" : "INFO") << "] " << result.title << '\n';
        if (!result.command.empty()) {
            std::cout << "  Команда      : " << result.command << '\n';
        }
        std::cout << "  Для чего     : " << result.purpose << '\n';
        std::cout << "  Примечание   : " << result.note << '\n';
        std::cout << "  Результат:\n";
        std::cout << "    ------------------------------\n";
        for (const auto& line : sec::splitLines(result.output)) {
            std::cout << "    " << line << '\n';
        }
        std::cout << "    ------------------------------\n\n";
    }
}

std::optional<std::filesystem::path> parsePathFlag(const std::vector<std::string>& args, const std::string_view flag) {
    for (std::size_t index = 0; index + 1 < args.size(); ++index) {
        if (args[index] == flag) {
            return std::filesystem::path(args[index + 1]);
        }
    }
    return std::nullopt;
}

bool hasFlag(const std::vector<std::string>& args, const std::string_view flag) {
    for (const auto& arg : args) {
        if (arg == flag) {
            return true;
        }
    }
    return false;
}

}  // namespace

namespace sec {

int Application::run(int argc, char** argv) {
    std::vector<std::string> args;
    args.reserve(static_cast<std::size_t>(argc > 1 ? argc - 1 : 0));
    for (int index = 1; index < argc; ++index) {
        args.emplace_back(argv[index]);
    }

    if (args.empty() || args.front() == "help" || args.front() == "--help" || args.front() == "-h") {
        printHelp();
        return 0;
    }

    const auto reportPath = parsePathFlag(args, "--report");
    const auto jsonPath = parsePathFlag(args, "--json");

    try {
        if (args.front() == "audit") {
            return runAudit(args, reportPath, jsonPath);
        }
        if (args.front() == "cleanup") {
            return runCleanup(args, reportPath, jsonPath);
        }
        if (args.front() == "learn") {
            return runLearn(reportPath, jsonPath, false);
        }
        if (args.front() == "study") {
            return runLearn(reportPath, jsonPath, true);
        }
        if (args.front() == "practice") {
            return runPractice(args, reportPath, jsonPath);
        }
        if (args.front() == "full") {
            return runFull(args, reportPath, jsonPath);
        }
    } catch (const std::exception& error) {
        std::cerr << "Ошибка выполнения: " << error.what() << '\n';
        return 1;
    }

    printHelp();
    return 1;
}

int Application::runAudit(const std::vector<std::string>& args,
                          const std::optional<std::filesystem::path>& reportPath,
                          const std::optional<std::filesystem::path>& jsonPath) {
    const auto bundle = WorkbenchService {}.buildAuditBundle(hasFlag(args, "--with-commands"), hasFlag(args, "--with-study"));

    printAuditItems(bundle.auditItems);
    printCommands(bundle.commands);
    printPlaybooks(bundle.playbooks);
    printTools(bundle.toolReferences);
    printArtifactLocations(bundle.artifactLocations);
    printLearningSections(bundle.learningSections);
    writeOutputs(bundle, reportPath, jsonPath);

    return 0;
}

int Application::runCleanup(const std::vector<std::string>& args,
                            const std::optional<std::filesystem::path>& reportPath,
                            const std::optional<std::filesystem::path>& jsonPath) {
    const auto bundle = WorkbenchService {}.buildCleanupBundle(hasFlag(args, "--dry-run"));
    printCleanupSummary(bundle.cleanupSummary.value());
    writeOutputs(bundle, reportPath, jsonPath);

    return 0;
}

int Application::runFull(const std::vector<std::string>& args,
                         const std::optional<std::filesystem::path>& reportPath,
                         const std::optional<std::filesystem::path>& jsonPath) {
    const auto bundle = WorkbenchService {}.buildFullBundle(hasFlag(args, "--dry-run"));

    printAuditItems(bundle.auditItems);
    printCleanupSummary(bundle.cleanupSummary.value());
    printCommands(bundle.commands);
    printPlaybooks(bundle.playbooks);
    printTools(bundle.toolReferences);
    printArtifactLocations(bundle.artifactLocations);
    printLearningSections(bundle.learningSections);
    writeOutputs(bundle, reportPath, jsonPath);

    return 0;
}

int Application::runPractice(const std::vector<std::string>& args,
                             const std::optional<std::filesystem::path>& reportPath,
                             const std::optional<std::filesystem::path>& jsonPath) {
    const auto bundle = WorkbenchService {}.buildPracticeBundle(hasFlag(args, "--linux-live"));

    printCommands(bundle.commands);
    printPlaybooks(bundle.playbooks);
    printLiveResults(bundle.liveCommandResults);
    printTools(bundle.toolReferences);
    printArtifactLocations(bundle.artifactLocations);
    printLearningSections(bundle.learningSections);
    writeOutputs(bundle, reportPath, jsonPath);
    return 0;
}

int Application::runLearn(const std::optional<std::filesystem::path>& reportPath,
                          const std::optional<std::filesystem::path>& jsonPath,
                          const bool studyOnly) {
    const auto bundle = studyOnly ? WorkbenchService {}.buildStudyBundle() : WorkbenchService {}.buildLearnBundle();

    if (!studyOnly) {
        printCommands(bundle.commands);
    }
    printPlaybooks(bundle.playbooks);
    printTools(bundle.toolReferences);
    printArtifactLocations(bundle.artifactLocations);
    printLearningSections(bundle.learningSections);
    writeOutputs(bundle, reportPath, jsonPath);
    return 0;
}

void Application::writeOutputs(const ExecutionBundle& bundle,
                               const std::optional<std::filesystem::path>& reportPath,
                               const std::optional<std::filesystem::path>& jsonPath) const {
    if (reportPath.has_value()) {
        ReportWriter {}.write(reportPath.value(), bundle);
        std::cout << "Markdown-отчет сохранен в " << reportPath->string() << '\n';
    }
    if (jsonPath.has_value()) {
        JsonWriter {}.write(jsonPath.value(), bundle);
        std::cout << "JSON-отчет сохранен в " << jsonPath->string() << '\n';
    }

    const auto historyPath = HistoryStore {}.save(bundle);
    if (historyPath.has_value()) {
        std::cout << "История запуска сохранена в " << historyPath->string() << '\n';
    }
}

void Application::printHelp() const {
    std::cout
        << "WindowsSecurityWorkbench\n"
        << "Использование:\n"
        << "  WindowsSecurityWorkbench audit [--report report.md] [--json report.json] [--with-commands] [--with-study]\n"
        << "  WindowsSecurityWorkbench cleanup [--dry-run] [--report cleanup.md] [--json cleanup.json]\n"
        << "  WindowsSecurityWorkbench learn [--report learn.md] [--json learn.json]\n"
        << "  WindowsSecurityWorkbench study [--report study.md] [--json study.json]\n"
        << "  WindowsSecurityWorkbench practice [--linux-live] [--report practice.md] [--json practice.json]\n"
        << "  WindowsSecurityWorkbench full [--dry-run] [--report report.md] [--json report.json]\n\n"
        << "Инструмент делает defensive-аудит Windows, безопасную очистку, показывает команды по Windows/Linux\n"
        << "и отдает учебный roadmap, практические сценарии, live-проверки и раздел \"где что лежит\".\n"
        << "Инструмент не извлекает сохраненные пароли, хэши, токены и другие секреты.\n";
}

}  // namespace sec
