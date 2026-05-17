#pragma once

#include <stdexcept>
#include <string>

namespace vna::config {

class ConfigError : public std::runtime_error {
public:
    explicit ConfigError(const std::string& message) : std::runtime_error(message) {}
};

}  // namespace vna::config
