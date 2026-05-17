#pragma once

#include <string>

namespace vna::domain {

bool isValidAeTitle(const std::string& title) noexcept;

void requireValidAeTitle(const std::string& title,
                         const char* context = "AE title");

}  // namespace vna::domain
