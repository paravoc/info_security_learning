#pragma once

#include "core/AuditTypes.h"

#include <vector>

namespace sec {

class CommandCatalog {
public:
    std::vector<CommandReference> build() const;
};

}  // namespace sec

