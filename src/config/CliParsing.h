#pragma once

#include <cstdint>
#include <string>
#include <string_view>

namespace vna::config::internal {

std::string takeValue(int& index, int argc, char** argv, std::string_view option);

std::uint16_t parsePort(const std::string& text, std::string_view option);

std::uint32_t parsePositiveUint32(const std::string& text, std::string_view option);

}  // namespace vna::config::internal
