#include "core/AuditTypes.h"

namespace sec {

std::string toString(const AuditStatus status) {
    switch (status) {
        case AuditStatus::info:
            return "INFO";
        case AuditStatus::ok:
            return "OK";
        case AuditStatus::warning:
            return "WARN";
        case AuditStatus::error:
            return "ERROR";
    }

    return "UNKNOWN";
}

std::string toDisplayString(const AuditStatus status) {
    switch (status) {
        case AuditStatus::info:
            return "Сведения";
        case AuditStatus::ok:
            return "Норма";
        case AuditStatus::warning:
            return "Риск";
        case AuditStatus::error:
            return "Ошибка";
    }

    return "Неизвестно";
}

}  // namespace sec
