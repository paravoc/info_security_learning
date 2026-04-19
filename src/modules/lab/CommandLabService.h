#pragma once

#include "core/AuditTypes.h"

#include <vector>

namespace sec {

class CommandLabService {
public:
    std::vector<LiveCommandResult> run(const std::vector<CommandReference>& commands, bool includeLinuxLive) const;
};

}  // namespace sec
