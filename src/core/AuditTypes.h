#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace sec {

enum class AuditStatus {
    info,
    ok,
    warning,
    error,
};

struct AuditItem {
    std::string category;
    std::string title;
    AuditStatus status {AuditStatus::info};
    std::vector<std::string> details;
    std::string recommendation;
};

struct CleanupTargetResult {
    std::string target;
    bool success {false};
    bool skipped {false};
    std::uint64_t filesAffected {0};
    std::uint64_t bytesAffected {0};
    std::string message;
};

struct CleanupSummary {
    bool dryRun {true};
    std::vector<CleanupTargetResult> targets;
};

struct CommandReference {
    std::string platform;
    std::string category;
    std::string title;
    std::string command;
    std::string purpose;
    std::string caution;
    std::string exampleOutput;
    std::string interpretation;
    std::string relatedTool;
    std::string scenario;
    std::vector<std::string> checks;
    std::vector<std::string> nextCommands;
    bool runnableInsideApp {false};
};

struct ArtifactLocation {
    std::string platform;
    std::string category;
    std::string artifact;
    std::vector<std::string> locations;
    std::string inspectionCommand;
    std::string purpose;
};

struct SystemToolReference {
    std::string platform;
    std::string category;
    std::string title;
    std::string openTarget;
    std::string purpose;
    std::vector<std::string> useCases;
    std::vector<std::string> whatToCheck;
    std::vector<std::string> relatedCommands;
};

struct PracticePlaybook {
    std::string platform;
    std::string category;
    std::string title;
    std::string symptom;
    std::string goal;
    std::vector<std::string> steps;
    std::vector<std::string> commands;
    std::vector<std::string> expectedSignals;
    std::vector<std::string> relatedTools;
};

struct LiveCommandResult {
    std::string platform;
    std::string category;
    std::string title;
    std::string command;
    std::string purpose;
    bool success {false};
    std::string output;
    std::string note;
};

struct LearningSection {
    std::string audience;
    std::string title;
    std::vector<std::string> topics;
    std::vector<std::string> labs;
    std::vector<std::string> milestones;
};

struct ExecutionBundle {
    std::string scenario;
    std::vector<AuditItem> auditItems;
    std::optional<CleanupSummary> cleanupSummary;
    std::vector<CommandReference> commands;
    std::vector<ArtifactLocation> artifactLocations;
    std::vector<SystemToolReference> toolReferences;
    std::vector<PracticePlaybook> playbooks;
    std::vector<LiveCommandResult> liveCommandResults;
    std::vector<LearningSection> learningSections;
};

std::string toString(AuditStatus status);
std::string toDisplayString(AuditStatus status);

}  // namespace sec
