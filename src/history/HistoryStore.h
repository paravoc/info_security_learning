#pragma once

#include "core/AuditTypes.h"

#include <filesystem>
#include <optional>

namespace sec {

class HistoryStore {
public:
    std::optional<std::filesystem::path> save(const ExecutionBundle& bundle) const;
};

}  // namespace sec

