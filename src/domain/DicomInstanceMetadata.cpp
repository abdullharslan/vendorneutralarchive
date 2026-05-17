#include "domain/DicomInstanceMetadata.h"

#include "domain/DicomUid.h"

#include <stdexcept>
#include <string>
#include <utility>

namespace vna::domain {

namespace {

void requireValidUidField(const std::string& uid, const char* field) {
    try {
        requireValidDicomUid(uid, field);
    } catch (const std::invalid_argument& e) {
        throw std::invalid_argument(
            std::string{"DicomInstanceMetadata "} + e.what());
    }
}

void requireNonEmpty(const std::string& value, const char* field) {
    if (value.empty()) {
        throw std::invalid_argument(
            std::string{"DicomInstanceMetadata "} + field + " must not be empty");
    }
}

}  // namespace

DicomInstanceMetadata::DicomInstanceMetadata(
    std::string sopInstanceUid,
    std::string sopClassUid,
    std::string studyInstanceUid,
    std::string seriesInstanceUid,
    std::string patientId,
    std::string modality
)
    : sopInstanceUid_(std::move(sopInstanceUid)),
      sopClassUid_(std::move(sopClassUid)),
      studyInstanceUid_(std::move(studyInstanceUid)),
      seriesInstanceUid_(std::move(seriesInstanceUid)),
      patientId_(std::move(patientId)),
      modality_(std::move(modality)) {
    requireValidUidField(sopInstanceUid_,    "SOP Instance UID");
    requireValidUidField(sopClassUid_,       "SOP Class UID");
    requireValidUidField(studyInstanceUid_,  "Study Instance UID");
    requireValidUidField(seriesInstanceUid_, "Series Instance UID");
    requireNonEmpty(modality_, "Modality");
}

const std::string& DicomInstanceMetadata::sopInstanceUid()    const noexcept { return sopInstanceUid_; }
const std::string& DicomInstanceMetadata::sopClassUid()       const noexcept { return sopClassUid_; }
const std::string& DicomInstanceMetadata::studyInstanceUid()  const noexcept { return studyInstanceUid_; }
const std::string& DicomInstanceMetadata::seriesInstanceUid() const noexcept { return seriesInstanceUid_; }
const std::string& DicomInstanceMetadata::patientId()         const noexcept { return patientId_; }
const std::string& DicomInstanceMetadata::modality()          const noexcept { return modality_; }

bool operator==(const DicomInstanceMetadata& a, const DicomInstanceMetadata& b) noexcept {
    return a.sopInstanceUid_    == b.sopInstanceUid_
        && a.sopClassUid_       == b.sopClassUid_
        && a.studyInstanceUid_  == b.studyInstanceUid_
        && a.seriesInstanceUid_ == b.seriesInstanceUid_
        && a.patientId_         == b.patientId_
        && a.modality_          == b.modality_;
}
bool operator!=(const DicomInstanceMetadata& a, const DicomInstanceMetadata& b) noexcept { return !(a == b); }

}  // namespace vna::domain
