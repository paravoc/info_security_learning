#include "report/ReportWriter.h"

#include "core/StringUtils.h"
#include "core/TimeUtils.h"

#include <fstream>
#include <stdexcept>

namespace {

const char* codeFenceForPlatform(const std::string& platform) {
    return platform == "Linux" ? "bash" : "powershell";
}

void writeStringLines(std::ofstream& stream, const std::vector<std::string>& lines) {
    for (const auto& line : lines) {
        stream << "- " << line << '\n';
    }
}

void writeCommands(std::ofstream& stream, const std::vector<sec::CommandReference>& commands) {
    std::string currentPlatform;
    std::string currentCategory;

    for (const auto& command : commands) {
        if (command.platform != currentPlatform || command.category != currentCategory) {
            currentPlatform = command.platform;
            currentCategory = command.category;
            stream << "### [" << currentPlatform << "] " << currentCategory << "\n\n";
        }

        stream << "#### " << command.title << "\n\n";
        stream << "```" << codeFenceForPlatform(command.platform) << "\n" << command.command << "\n```\n\n";
        if (!command.relatedTool.empty()) {
            stream << "- Связанная оснастка: " << command.relatedTool << '\n';
        }
        if (!command.scenario.empty()) {
            stream << "- Когда использовать: " << command.scenario << '\n';
        }
        stream << "- Назначение: " << command.purpose << '\n';
        stream << "- Осторожно: " << command.caution << '\n';
        stream << "- Можно запускать внутри приложения: " << (command.runnableInsideApp ? "да" : "нет") << "\n\n";
        if (!command.exampleOutput.empty()) {
            stream << "Пример вывода:\n\n";
            stream << "```text\n" << command.exampleOutput << "\n```\n\n";
        }
        if (!command.interpretation.empty()) {
            stream << "- Как читать результат: " << command.interpretation << '\n';
        }
        if (!command.checks.empty()) {
            stream << "- На что смотреть:\n";
            writeStringLines(stream, command.checks);
        }
        if (!command.nextCommands.empty()) {
            stream << "- Что проверить потом:\n";
            writeStringLines(stream, command.nextCommands);
        }
        stream << '\n';
    }
}

void writeArtifactLocations(std::ofstream& stream, const std::vector<sec::ArtifactLocation>& locations) {
    std::string currentPlatform;
    std::string currentCategory;

    for (const auto& location : locations) {
        if (location.platform != currentPlatform || location.category != currentCategory) {
            currentPlatform = location.platform;
            currentCategory = location.category;
            stream << "### [" << currentPlatform << "] " << currentCategory << "\n\n";
        }

        stream << "#### " << location.artifact << "\n\n";
        stream << "- Где искать:\n";
        writeStringLines(stream, location.locations);
        stream << "- Команда проверки: `" << location.inspectionCommand << "`\n";
        stream << "- Зачем смотреть: " << location.purpose << "\n\n";
    }
}

void writeTools(std::ofstream& stream, const std::vector<sec::SystemToolReference>& tools) {
    std::string currentPlatform;
    std::string currentCategory;

    for (const auto& tool : tools) {
        if (tool.platform != currentPlatform || tool.category != currentCategory) {
            currentPlatform = tool.platform;
            currentCategory = tool.category;
            stream << "### [" << currentPlatform << "] " << currentCategory << "\n\n";
        }

        stream << "#### " << tool.title << "\n\n";
        stream << "- Где открыть: `" << tool.openTarget << "`\n";
        stream << "- Для чего нужен: " << tool.purpose << "\n";
        stream << "- Когда открывать:\n";
        writeStringLines(stream, tool.useCases);
        stream << "- Что смотреть:\n";
        writeStringLines(stream, tool.whatToCheck);
        stream << "- Какие команды помогают:\n";
        writeStringLines(stream, tool.relatedCommands);
        stream << '\n';
    }
}

void writePlaybooks(std::ofstream& stream, const std::vector<sec::PracticePlaybook>& playbooks) {
    std::string currentPlatform;
    std::string currentCategory;

    for (const auto& playbook : playbooks) {
        if (playbook.platform != currentPlatform || playbook.category != currentCategory) {
            currentPlatform = playbook.platform;
            currentCategory = playbook.category;
            stream << "### [" << currentPlatform << "] " << currentCategory << "\n\n";
        }

        stream << "#### " << playbook.title << "\n\n";
        stream << "- Симптом: " << playbook.symptom << '\n';
        stream << "- Цель: " << playbook.goal << '\n';
        stream << "- Что сделать:\n";
        writeStringLines(stream, playbook.steps);
        stream << "- Команды:\n";
        writeStringLines(stream, playbook.commands);
        stream << "- На что смотреть:\n";
        writeStringLines(stream, playbook.expectedSignals);
        if (!playbook.relatedTools.empty()) {
            stream << "- Куда еще зайти:\n";
            writeStringLines(stream, playbook.relatedTools);
        }
        stream << '\n';
    }
}

void writeLiveResults(std::ofstream& stream, const std::vector<sec::LiveCommandResult>& results) {
    std::string currentPlatform;
    std::string currentCategory;

    for (const auto& result : results) {
        if (result.platform != currentPlatform || result.category != currentCategory) {
            currentPlatform = result.platform;
            currentCategory = result.category;
            stream << "### [" << currentPlatform << "] " << currentCategory << "\n\n";
        }

        stream << "#### " << result.title << "\n\n";
        if (!result.command.empty()) {
            stream << "```" << codeFenceForPlatform(result.platform) << "\n" << result.command << "\n```\n\n";
        }
        stream << "- Статус: " << (result.success ? "OK" : "INFO/ERROR") << '\n';
        stream << "- Для чего: " << result.purpose << '\n';
        stream << "- Примечание: " << result.note << "\n\n";
        stream << "```text\n" << result.output << "\n```\n\n";
    }
}

}  // namespace

namespace sec {

void ReportWriter::write(const std::filesystem::path& path, const ExecutionBundle& bundle) const {
    std::ofstream stream(path, std::ios::binary);
    if (!stream.is_open()) {
        throw std::runtime_error("Unable to open report file for writing.");
    }

    stream << "\xEF\xBB\xBF";
    stream << "# Windows Security Workbench\n\n";
    stream << "Сформировано: " << currentTimestampHuman() << "\n\n";
    stream << "Сценарий: " << bundle.scenario << "\n\n";

    if (!bundle.auditItems.empty()) {
        stream << "## Находки аудита\n\n";
        for (const auto& item : bundle.auditItems) {
            stream << "### [" << toDisplayString(item.status) << "] " << item.category << " - " << item.title << "\n\n";
            writeStringLines(stream, item.details);
            if (!item.recommendation.empty()) {
                stream << "- Рекомендация: " << item.recommendation << '\n';
            }
            stream << '\n';
        }
    }

    if (bundle.cleanupSummary.has_value()) {
        stream << "## Очистка и кэш\n\n";
        stream << "- Режим: " << (bundle.cleanupSummary->dryRun ? "dry-run" : "применение") << '\n';
        for (const auto& target : bundle.cleanupSummary->targets) {
            stream << "- " << target.target
                   << " | files=" << target.filesAffected
                   << " | bytes=" << humanReadableBytes(target.bytesAffected)
                   << " | message=" << target.message << '\n';
        }
        stream << '\n';
    }

    if (!bundle.commands.empty()) {
        stream << "## Команды и шпаргалки\n\n";
        writeCommands(stream, bundle.commands);
    }

    if (!bundle.toolReferences.empty()) {
        stream << "## Системные инструменты и оснастки\n\n";
        writeTools(stream, bundle.toolReferences);
    }

    if (!bundle.playbooks.empty()) {
        stream << "## Практические сценарии\n\n";
        writePlaybooks(stream, bundle.playbooks);
    }

    if (!bundle.liveCommandResults.empty()) {
        stream << "## Живые результаты команд\n\n";
        writeLiveResults(stream, bundle.liveCommandResults);
    }

    if (!bundle.artifactLocations.empty()) {
        stream << "## Где искать артефакты\n\n";
        writeArtifactLocations(stream, bundle.artifactLocations);
    }

    if (!bundle.learningSections.empty()) {
        stream << "## План обучения\n\n";
        for (const auto& section : bundle.learningSections) {
            stream << "### " << section.audience << " - " << section.title << "\n\n";
            stream << "#### Что изучить\n\n";
            writeStringLines(stream, section.topics);
            stream << "\n#### Практика\n\n";
            writeStringLines(stream, section.labs);
            stream << "\n#### Цели\n\n";
            writeStringLines(stream, section.milestones);
            stream << '\n';
        }
    }
}

}  // namespace sec
