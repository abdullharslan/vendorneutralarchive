#pragma once

#include <string>

namespace vna::domain {

class DicomInstanceMetadata {
public:
    DicomInstanceMetadata(
        std::string sopInstanceUid,
        std::string sopClassUid,
        std::string studyInstanceUid,
        std::string seriesInstanceUid,
        std::string patientId,
        std::string modality
    );

    const std::string& sopInstanceUid() const noexcept;
    const std::string& sopClassUid() const noexcept;
    const std::string& studyInstanceUid() const noexcept;
    const std::string& seriesInstanceUid() const noexcept;
    const std::string& patientId() const noexcept;
    const std::string& modality() const noexcept;

    friend bool operator==(const DicomInstanceMetadata&, const DicomInstanceMetadata&) noexcept;
    friend bool operator!=(const DicomInstanceMetadata&, const DicomInstanceMetadata&) noexcept;

private:
    std::string sopInstanceUid_;
    std::string sopClassUid_;
    std::string studyInstanceUid_;
    std::string seriesInstanceUid_;
    std::string patientId_;
    std::string modality_;
};

}  // namespace vna::domain
