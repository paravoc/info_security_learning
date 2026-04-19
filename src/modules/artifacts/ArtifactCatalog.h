#pragma once

#include "core/AuditTypes.h"

#include <vector>

namespace sec {

class ArtifactCatalog {
public:
    std::vector<ArtifactLocation> build() const;
};

}  // namespace sec
