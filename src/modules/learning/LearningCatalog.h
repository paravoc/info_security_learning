#pragma once

#include "core/AuditTypes.h"

#include <vector>

namespace sec {

class LearningCatalog {
public:
    std::vector<LearningSection> build() const;
};

}  // namespace sec

