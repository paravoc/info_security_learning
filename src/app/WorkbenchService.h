#pragma once

#include "core/AuditTypes.h"

namespace sec {

class WorkbenchService {
public:
    ExecutionBundle buildAuditBundle(bool includeCommands, bool includeStudy) const;
    ExecutionBundle buildCleanupBundle(bool dryRun) const;
    ExecutionBundle buildFullBundle(bool dryRun) const;
    ExecutionBundle buildLearnBundle() const;
    ExecutionBundle buildStudyBundle() const;
    ExecutionBundle buildPracticeBundle(bool includeLinuxLive) const;
};

}  // namespace sec
