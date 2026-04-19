#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace sec {

std::string humanReadableBytes(std::uint64_t bytes);
std::string humanReadableDurationSeconds(std::uint64_t seconds);
std::string formatBool(bool value, std::string_view trueLabel, std::string_view falseLabel);
std::string trim(std::string value);
std::vector<std::string> split(std::string_view value, char delimiter);
std::vector<std::string> splitLines(const std::string& value);
std::string escapeJson(std::string_view value);

}  // namespace sec
