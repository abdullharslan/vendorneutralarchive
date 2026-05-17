#pragma once

#include "domain/DicomFile.h"
#include "domain/DicomInstanceMetadata.h"

#include <string>

namespace vna::dicom {

struct DicomFileReadResult {
    domain::DicomFile file;
    domain::DicomInstanceMetadata metadata;
};

class DicomFileReader {
public:
    DicomFileReadResult read(const std::string& path) const;
};

}  // namespace vna::dicom
