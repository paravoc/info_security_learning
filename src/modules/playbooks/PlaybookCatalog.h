#pragma once

#include "core/AuditTypes.h"

#include <vector>

namespace sec {

class PlaybookCatalog {
public:
    std::vector<PracticePlaybook> build() const;
};

}  // namespace sec
