#include "config/CliParsing.h"

#include "config/ConfigError.h"

#include <charconv>
#include <string>
#include <system_error>

namespace vna::config::internal {

std::string takeValue(int& index, int argc, char** argv, std::string_view option) {
    if (index + 1 >= argc) {
        throw ConfigError(std::string{option} + " requires a value");
    }
    ++index;
    return std::string{argv[index]};
}

std::uint16_t parsePort(const std::string& text, std::string_view option) {
    std::uint32_t value = 0;
    const char* begin = text.data();
    const char* end = text.data() + text.size();
    auto result = std::from_chars(begin, end, value);
    if (result.ec != std::errc{} || result.ptr != end || value == 0 || value > 65535) {
        throw ConfigError(std::string{option} +
                          " expects an integer in the range 1-65535, got \"" + text + "\"");
    }
    return static_cast<std::uint16_t>(value);
}

std::uint32_t parsePositiveUint32(const std::string& text, std::string_view option) {
    std::uint32_t value = 0;
    const char* begin = text.data();
    const char* end = text.data() + text.size();
    auto result = std::from_chars(begin, end, value);
    if (result.ec != std::errc{} || result.ptr != end || value == 0) {
        throw ConfigError(std::string{option} +
                          " expects a positive integer, got \"" + text + "\"");
    }
    return value;
}

}  // namespace vna::config::internal
