#pragma once

#include "core/AuditTypes.h"

#include <filesystem>

namespace sec {

class JsonWriter {
public:
    std::string stringify(const ExecutionBundle& bundle) const;
    void write(const std::filesystem::path& path, const ExecutionBundle& bundle) const;
};

}  // namespace sec
