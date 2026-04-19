#pragma once

#include "core/AuditTypes.h"

namespace sec {

class CleanupService {
public:
    CleanupSummary run(bool dryRun) const;
};

}  // namespace sec

