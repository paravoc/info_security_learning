#include "core/StringUtils.h"

#include <array>
#include <cmath>
#include <cctype>
#include <iomanip>
#include <sstream>

namespace sec {

std::string humanReadableBytes(const std::uint64_t bytes) {
    static constexpr std::array<const char*, 5> kUnits {"B", "KB", "MB", "GB", "TB"};

    double value = static_cast<double>(bytes);
    std::size_t unitIndex = 0;
    while (value >= 1024.0 && unitIndex + 1 < kUnits.size()) {
        value /= 1024.0;
        ++unitIndex;
    }

    std::ostringstream stream;
    stream.setf(std::ios::fixed);
    stream.precision(unitIndex == 0 ? 0 : 2);
    stream << value << ' ' << kUnits[unitIndex];
    return stream.str();
}

std::string humanReadableDurationSeconds(const std::uint64_t seconds) {
    constexpr std::uint64_t kMinute = 60;
    constexpr std::uint64_t kHour = 60 * kMinute;
    constexpr std::uint64_t kDay = 24 * kHour;

    const auto days = seconds / kDay;
    const auto hours = (seconds % kDay) / kHour;
    const auto minutes = (seconds % kHour) / kMinute;

    std::ostringstream stream;
    if (days > 0) {
        stream << days << "d ";
    }
    if (hours > 0 || days > 0) {
        stream << hours << "h ";
    }
    stream << minutes << "m";
    return stream.str();
}

std::string formatBool(const bool value, const std::string_view trueLabel, const std::string_view falseLabel) {
    return value ? std::string(trueLabel) : std::string(falseLabel);
}

std::string trim(std::string value) {
    while (!value.empty() && std::isspace(static_cast<unsigned char>(value.front())) != 0) {
        value.erase(value.begin());
    }

    while (!value.empty() && std::isspace(static_cast<unsigned char>(value.back())) != 0) {
        value.pop_back();
    }

    return value;
}

std::vector<std::string> split(const std::string_view value, const char delimiter) {
    std::vector<std::string> parts;
    std::string current;
    for (const char character : value) {
        if (character == delimiter) {
            parts.push_back(current);
            current.clear();
            continue;
        }
        current.push_back(character);
    }
    parts.push_back(current);
    return parts;
}

std::vector<std::string> splitLines(const std::string& value) {
    std::vector<std::string> lines;
    std::string current;
    for (const char character : value) {
        if (character == '\r') {
            continue;
        }
        if (character == '\n') {
            lines.push_back(current);
            current.clear();
            continue;
        }
        current.push_back(character);
    }
    if (!current.empty()) {
        lines.push_back(current);
    }
    return lines;
}

std::string escapeJson(const std::string_view value) {
    std::ostringstream stream;
    for (const char character : value) {
        switch (character) {
            case '\\':
                stream << "\\\\";
                break;
            case '"':
                stream << "\\\"";
                break;
            case '\b':
                stream << "\\b";
                break;
            case '\f':
                stream << "\\f";
                break;
            case '\n':
                stream << "\\n";
                break;
            case '\r':
                stream << "\\r";
                break;
            case '\t':
                stream << "\\t";
                break;
            default:
                if (static_cast<unsigned char>(character) < 0x20) {
                    stream << "\\u"
                           << std::hex
                           << std::uppercase
                           << std::setw(4)
                           << std::setfill('0')
                           << static_cast<int>(static_cast<unsigned char>(character))
                           << std::dec
                           << std::nouppercase;
                } else {
                    stream << character;
                }
                break;
        }
    }

    return stream.str();
}

}  // namespace sec
