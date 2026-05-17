#pragma once

#include <stdexcept>
#include <string>

namespace vna::dicom {

class DicomFileReadError : public std::runtime_error {
public:
    explicit DicomFileReadError(const std::string& message) : std::runtime_error(message) {}
};

}  // namespace vna::dicom
