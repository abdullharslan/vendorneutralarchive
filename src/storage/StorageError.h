#pragma once

#include <stdexcept>
#include <string>

namespace vna::storage {

class StorageError : public std::runtime_error {
public:
    explicit StorageError(const std::string& message) : std::runtime_error(message) {}
};

}  // namespace vna::storage
