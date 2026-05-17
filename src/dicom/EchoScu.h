#pragma once

#include "domain/DicomNode.h"
#include "domain/EchoResult.h"

#include <cstdint>
#include <string>

namespace vna::dicom {

class EchoScu {
public:
    domain::EchoResult echo(const std::string& callingAe,
                            const domain::DicomNode& peer,
                            std::uint32_t timeoutSeconds) const;
};

}  // namespace vna::dicom
