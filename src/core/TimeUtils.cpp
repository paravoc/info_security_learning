#include "core/TimeUtils.h"

#include <chrono>
#include <iomanip>
#include <sstream>

namespace {

std::string formatNow(const char* format) {
    const auto now = std::chrono::system_clock::now();
    const auto value = std::chrono::system_clock::to_time_t(now);
    std::tm localTime {};
    localtime_s(&localTime, &value);

    std::ostringstream stream;
    stream << std::put_time(&localTime, format);
    return stream.str();
}

}  // namespace

namespace sec {

std::string currentTimestampHuman() {
    return formatNow("%Y-%m-%d %H:%M:%S");
}

std::string currentTimestampCompact() {
    return formatNow("%Y%m%d_%H%M%S");
}

}  // namespace sec

