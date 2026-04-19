#pragma once

#include "core/AuditTypes.h"

#include <vector>

namespace sec {

class AccountsAuditModule {
public:
    std::vector<AuditItem> collect() const;
};

}  // namespace sec

