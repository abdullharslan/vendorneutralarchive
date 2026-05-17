#pragma once

#include <stdexcept>
#include <string>

namespace vna::dicom {

class EchoScuError : public std::runtime_error {
public:
    explicit EchoScuError(const std::string& message) : std::runtime_error(message) {}
};

}  // namespace vna::dicom
