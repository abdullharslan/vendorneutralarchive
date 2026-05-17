#pragma once

#include <stdexcept>
#include <string>

namespace vna::dicom {

class StoreScpError : public std::runtime_error {
public:
    explicit StoreScpError(const std::string& message) : std::runtime_error(message) {}
};

}  // namespace vna::dicom
