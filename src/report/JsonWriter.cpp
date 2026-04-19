#include "report/JsonWriter.h"

#include "core/StringUtils.h"
#include "core/TimeUtils.h"

#include <fstream>
#include <sstream>
#include <stdexcept>

namespace {

void writeStringArray(std::ostream& stream, const std::vector<std::string>& values, const int indentLevel) {
    stream << "[";
    if (!values.empty()) {
        stream << "\n";
    }

    for (std::size_t index = 0; index < values.size(); ++index) {
        stream << std::string(static_cast<std::size_t>(indentLevel), ' ')
               << "\"" << sec::escapeJson(values[index]) << "\"";
        if (index + 1 < values.size()) {
            stream << ",";
        }
        stream << "\n";
    }

    if (!values.empty()) {
        stream << std::string(static_cast<std::size_t>(indentLevel - 2), ' ');
    }
    stream << "]";
}

}  // namespace

namespace sec {

std::string JsonWriter::stringify(const ExecutionBundle& bundle) const {
    std::ostringstream stream;
    stream << "{\n";
    stream << "  \"generatedAt\": \"" << escapeJson(currentTimestampHuman()) << "\",\n";
    stream << "  \"scenario\": \"" << escapeJson(bundle.scenario) << "\",\n";

    stream << "  \"auditItems\": [\n";
    for (std::size_t index = 0; index < bundle.auditItems.size(); ++index) {
        const auto& item = bundle.auditItems[index];
        stream << "    {\n";
        stream << "      \"category\": \"" << escapeJson(item.category) << "\",\n";
        stream << "      \"title\": \"" << escapeJson(item.title) << "\",\n";
        stream << "      \"status\": \"" << escapeJson(toString(item.status)) << "\",\n";
        stream << "      \"details\": ";
        writeStringArray(stream, item.details, 8);
        stream << ",\n";
        stream << "      \"recommendation\": \"" << escapeJson(item.recommendation) << "\"\n";
        stream << "    }";
        if (index + 1 < bundle.auditItems.size()) {
            stream << ",";
        }
        stream << "\n";
    }
    stream << "  ],\n";

    stream << "  \"cleanupSummary\": ";
    if (bundle.cleanupSummary.has_value()) {
        stream << "{\n";
        stream << "    \"dryRun\": " << (bundle.cleanupSummary->dryRun ? "true" : "false") << ",\n";
        stream << "    \"targets\": [\n";
        for (std::size_t index = 0; index < bundle.cleanupSummary->targets.size(); ++index) {
            const auto& target = bundle.cleanupSummary->targets[index];
            stream << "      {\n";
            stream << "        \"target\": \"" << escapeJson(target.target) << "\",\n";
            stream << "        \"success\": " << (target.success ? "true" : "false") << ",\n";
            stream << "        \"skipped\": " << (target.skipped ? "true" : "false") << ",\n";
            stream << "        \"filesAffected\": " << target.filesAffected << ",\n";
            stream << "        \"bytesAffected\": " << target.bytesAffected << ",\n";
            stream << "        \"message\": \"" << escapeJson(target.message) << "\"\n";
            stream << "      }";
            if (index + 1 < bundle.cleanupSummary->targets.size()) {
                stream << ",";
            }
            stream << "\n";
        }
        stream << "    ]\n";
        stream << "  },\n";
    } else {
        stream << "null,\n";
    }

    stream << "  \"commands\": [\n";
    for (std::size_t index = 0; index < bundle.commands.size(); ++index) {
        const auto& command = bundle.commands[index];
        stream << "    {\n";
        stream << "      \"platform\": \"" << escapeJson(command.platform) << "\",\n";
        stream << "      \"category\": \"" << escapeJson(command.category) << "\",\n";
        stream << "      \"title\": \"" << escapeJson(command.title) << "\",\n";
        stream << "      \"command\": \"" << escapeJson(command.command) << "\",\n";
        stream << "      \"purpose\": \"" << escapeJson(command.purpose) << "\",\n";
        stream << "      \"caution\": \"" << escapeJson(command.caution) << "\",\n";
        stream << "      \"exampleOutput\": \"" << escapeJson(command.exampleOutput) << "\",\n";
        stream << "      \"interpretation\": \"" << escapeJson(command.interpretation) << "\",\n";
        stream << "      \"relatedTool\": \"" << escapeJson(command.relatedTool) << "\",\n";
        stream << "      \"scenario\": \"" << escapeJson(command.scenario) << "\",\n";
        stream << "      \"checks\": ";
        writeStringArray(stream, command.checks, 8);
        stream << ",\n";
        stream << "      \"nextCommands\": ";
        writeStringArray(stream, command.nextCommands, 8);
        stream << ",\n";
        stream << "      \"runnableInsideApp\": " << (command.runnableInsideApp ? "true" : "false") << "\n";
        stream << "    }";
        if (index + 1 < bundle.commands.size()) {
            stream << ",";
        }
        stream << "\n";
    }
    stream << "  ],\n";

    stream << "  \"artifactLocations\": [\n";
    for (std::size_t index = 0; index < bundle.artifactLocations.size(); ++index) {
        const auto& location = bundle.artifactLocations[index];
        stream << "    {\n";
        stream << "      \"platform\": \"" << escapeJson(location.platform) << "\",\n";
        stream << "      \"category\": \"" << escapeJson(location.category) << "\",\n";
        stream << "      \"artifact\": \"" << escapeJson(location.artifact) << "\",\n";
        stream << "      \"locations\": ";
        writeStringArray(stream, location.locations, 8);
        stream << ",\n";
        stream << "      \"inspectionCommand\": \"" << escapeJson(location.inspectionCommand) << "\",\n";
        stream << "      \"purpose\": \"" << escapeJson(location.purpose) << "\"\n";
        stream << "    }";
        if (index + 1 < bundle.artifactLocations.size()) {
            stream << ",";
        }
        stream << "\n";
    }
    stream << "  ],\n";

    stream << "  \"toolReferences\": [\n";
    for (std::size_t index = 0; index < bundle.toolReferences.size(); ++index) {
        const auto& tool = bundle.toolReferences[index];
        stream << "    {\n";
        stream << "      \"platform\": \"" << escapeJson(tool.platform) << "\",\n";
        stream << "      \"category\": \"" << escapeJson(tool.category) << "\",\n";
        stream << "      \"title\": \"" << escapeJson(tool.title) << "\",\n";
        stream << "      \"openTarget\": \"" << escapeJson(tool.openTarget) << "\",\n";
        stream << "      \"purpose\": \"" << escapeJson(tool.purpose) << "\",\n";
        stream << "      \"useCases\": ";
        writeStringArray(stream, tool.useCases, 8);
        stream << ",\n";
        stream << "      \"whatToCheck\": ";
        writeStringArray(stream, tool.whatToCheck, 8);
        stream << ",\n";
        stream << "      \"relatedCommands\": ";
        writeStringArray(stream, tool.relatedCommands, 8);
        stream << "\n";
        stream << "    }";
        if (index + 1 < bundle.toolReferences.size()) {
            stream << ",";
        }
        stream << "\n";
    }
    stream << "  ],\n";

    stream << "  \"playbooks\": [\n";
    for (std::size_t index = 0; index < bundle.playbooks.size(); ++index) {
        const auto& playbook = bundle.playbooks[index];
        stream << "    {\n";
        stream << "      \"platform\": \"" << escapeJson(playbook.platform) << "\",\n";
        stream << "      \"category\": \"" << escapeJson(playbook.category) << "\",\n";
        stream << "      \"title\": \"" << escapeJson(playbook.title) << "\",\n";
        stream << "      \"symptom\": \"" << escapeJson(playbook.symptom) << "\",\n";
        stream << "      \"goal\": \"" << escapeJson(playbook.goal) << "\",\n";
        stream << "      \"steps\": ";
        writeStringArray(stream, playbook.steps, 8);
        stream << ",\n";
        stream << "      \"commands\": ";
        writeStringArray(stream, playbook.commands, 8);
        stream << ",\n";
        stream << "      \"expectedSignals\": ";
        writeStringArray(stream, playbook.expectedSignals, 8);
        stream << ",\n";
        stream << "      \"relatedTools\": ";
        writeStringArray(stream, playbook.relatedTools, 8);
        stream << "\n";
        stream << "    }";
        if (index + 1 < bundle.playbooks.size()) {
            stream << ",";
        }
        stream << "\n";
    }
    stream << "  ],\n";

    stream << "  \"liveCommandResults\": [\n";
    for (std::size_t index = 0; index < bundle.liveCommandResults.size(); ++index) {
        const auto& result = bundle.liveCommandResults[index];
        stream << "    {\n";
        stream << "      \"platform\": \"" << escapeJson(result.platform) << "\",\n";
        stream << "      \"category\": \"" << escapeJson(result.category) << "\",\n";
        stream << "      \"title\": \"" << escapeJson(result.title) << "\",\n";
        stream << "      \"command\": \"" << escapeJson(result.command) << "\",\n";
        stream << "      \"purpose\": \"" << escapeJson(result.purpose) << "\",\n";
        stream << "      \"success\": " << (result.success ? "true" : "false") << ",\n";
        stream << "      \"output\": \"" << escapeJson(result.output) << "\",\n";
        stream << "      \"note\": \"" << escapeJson(result.note) << "\"\n";
        stream << "    }";
        if (index + 1 < bundle.liveCommandResults.size()) {
            stream << ",";
        }
        stream << "\n";
    }
    stream << "  ],\n";

    stream << "  \"learningSections\": [\n";
    for (std::size_t index = 0; index < bundle.learningSections.size(); ++index) {
        const auto& section = bundle.learningSections[index];
        stream << "    {\n";
        stream << "      \"audience\": \"" << escapeJson(section.audience) << "\",\n";
        stream << "      \"title\": \"" << escapeJson(section.title) << "\",\n";
        stream << "      \"topics\": ";
        writeStringArray(stream, section.topics, 8);
        stream << ",\n";
        stream << "      \"labs\": ";
        writeStringArray(stream, section.labs, 8);
        stream << ",\n";
        stream << "      \"milestones\": ";
        writeStringArray(stream, section.milestones, 8);
        stream << "\n";
        stream << "    }";
        if (index + 1 < bundle.learningSections.size()) {
            stream << ",";
        }
        stream << "\n";
    }
    stream << "  ]\n";
    stream << "}\n";
    return stream.str();
}

void JsonWriter::write(const std::filesystem::path& path, const ExecutionBundle& bundle) const {
    std::ofstream stream(path, std::ios::binary);
    if (!stream.is_open()) {
        throw std::runtime_error("Unable to open JSON file for writing.");
    }

    stream << stringify(bundle);
}

}  // namespace sec
