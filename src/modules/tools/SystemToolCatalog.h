#pragma once

#include "core/AuditTypes.h"

#include <vector>

namespace sec {

class SystemToolCatalog {
public:
    std::vector<SystemToolReference> build() const;
};

}  // namespace sec
