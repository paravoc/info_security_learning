#pragma once

#include <filesystem>

#include "core/AuditTypes.h"

namespace sec {

class ReportWriter {
public:
    void write(const std::filesystem::path& path, const ExecutionBundle& bundle) const;
};

}  // namespace sec
