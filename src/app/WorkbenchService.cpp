#include "app/WorkbenchService.h"

#include "modules/accounts/AccountsAuditModule.h"
#include "modules/artifacts/ArtifactCatalog.h"
#include "modules/cleanup/CleanupService.h"
#include "modules/commands/CommandCatalog.h"
#include "modules/firewall/FirewallAuditModule.h"
#include "modules/lab/CommandLabService.h"
#include "modules/learning/LearningCatalog.h"
#include "modules/network/NetworkAuditModule.h"
#include "modules/playbooks/PlaybookCatalog.h"
#include "modules/services/ServicesAuditModule.h"
#include "modules/system/SystemAuditModule.h"
#include "modules/tasks/ScheduledTasksAuditModule.h"
#include "modules/tools/SystemToolCatalog.h"
#include "modules/updates/UpdatesAuditModule.h"

namespace {

std::vector<sec::AuditItem> collectAuditItems() {
    std::vector<sec::AuditItem> items;

    const sec::SystemAuditModule systemModule;
    const sec::AccountsAuditModule accountsModule;
    const sec::NetworkAuditModule networkModule;
    const sec::FirewallAuditModule firewallModule;
    const sec::ServicesAuditModule servicesModule;
    const sec::UpdatesAuditModule updatesModule;
    const sec::ScheduledTasksAuditModule tasksModule;

    for (const auto& moduleItems : {systemModule.collect(),
                                    accountsModule.collect(),
                                    networkModule.collect(),
                                    firewallModule.collect(),
                                    servicesModule.collect(),
                                    updatesModule.collect(),
                                    tasksModule.collect()}) {
        items.insert(items.end(), moduleItems.begin(), moduleItems.end());
    }

    return items;
}

void attachLearningReferences(sec::ExecutionBundle& bundle, const bool includeCommands, const bool includeStudy) {
    if (includeCommands) {
        bundle.commands = sec::CommandCatalog {}.build();
    }

    if (includeStudy || includeCommands) {
        bundle.artifactLocations = sec::ArtifactCatalog {}.build();
        bundle.toolReferences = sec::SystemToolCatalog {}.build();
        bundle.playbooks = sec::PlaybookCatalog {}.build();
    }

    if (includeStudy) {
        bundle.learningSections = sec::LearningCatalog {}.build();
    }
}

}  // namespace

namespace sec {

ExecutionBundle WorkbenchService::buildAuditBundle(const bool includeCommands, const bool includeStudy) const {
    ExecutionBundle bundle;
    bundle.scenario = "audit";
    bundle.auditItems = collectAuditItems();
    attachLearningReferences(bundle, includeCommands, includeStudy);
    return bundle;
}

ExecutionBundle WorkbenchService::buildCleanupBundle(const bool dryRun) const {
    ExecutionBundle bundle;
    bundle.scenario = "cleanup";
    bundle.cleanupSummary = CleanupService {}.run(dryRun);
    return bundle;
}

ExecutionBundle WorkbenchService::buildFullBundle(const bool dryRun) const {
    ExecutionBundle bundle;
    bundle.scenario = "full";
    bundle.auditItems = collectAuditItems();
    bundle.cleanupSummary = CleanupService {}.run(dryRun);
    attachLearningReferences(bundle, true, true);
    return bundle;
}

ExecutionBundle WorkbenchService::buildLearnBundle() const {
    ExecutionBundle bundle;
    bundle.scenario = "learn";
    attachLearningReferences(bundle, true, true);
    return bundle;
}

ExecutionBundle WorkbenchService::buildStudyBundle() const {
    ExecutionBundle bundle;
    bundle.scenario = "study";
    attachLearningReferences(bundle, false, true);
    return bundle;
}

ExecutionBundle WorkbenchService::buildPracticeBundle(const bool includeLinuxLive) const {
    ExecutionBundle bundle;
    bundle.scenario = "practice";
    attachLearningReferences(bundle, true, true);
    bundle.liveCommandResults = CommandLabService {}.run(bundle.commands, includeLinuxLive);
    return bundle;
}

}  // namespace sec
