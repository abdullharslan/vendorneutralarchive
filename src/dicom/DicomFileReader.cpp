#include "dicom/DicomFileReader.h"

#include "dicom/DicomFileReadError.h"

#include <dcmtk/dcmdata/dcdeftag.h>
#include <dcmtk/dcmdata/dcfilefo.h>
#include <dcmtk/ofstd/ofcond.h>
#include <dcmtk/ofstd/ofstring.h>

#include <cstdint>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <system_error>
#include <utility>

namespace vna::dicom {

namespace {

std::string requireString(DcmDataset& dataset, const DcmTagKey& tag, const char* label) {
    OFString value;
    const OFCondition status = dataset.findAndGetOFString(tag, value);
    if (status.bad() || value.empty()) {
        throw DicomFileReadError(std::string{"DICOM dataset is missing required "} + label);
    }
    return std::string{value.c_str(), value.length()};
}

std::string optionalString(DcmDataset& dataset, const DcmTagKey& tag) {
    OFString value;
    dataset.findAndGetOFString(tag, value);
    return std::string{value.c_str(), value.length()};
}

}  // namespace

DicomFileReadResult DicomFileReader::read(const std::string& path) const {
    if (path.empty()) {
        throw DicomFileReadError("DICOM file path must not be empty");
    }

    namespace fs = std::filesystem;
    const fs::path filePath{path};
    std::error_code ec;

    if (!fs::exists(filePath, ec) || ec) {
        throw DicomFileReadError("DICOM file does not exist: " + path);
    }
    if (fs::is_directory(filePath, ec)) {
        throw DicomFileReadError("DICOM path is a directory, not a file: " + path);
    }

    const std::uintmax_t size = fs::file_size(filePath, ec);
    if (ec) {
        throw DicomFileReadError("Failed to read DICOM file size for " + path + ": " + ec.message());
    }
    if (size == 0) {
        throw DicomFileReadError("DICOM file is empty: " + path);
    }

    DcmFileFormat fileFormat;
    const OFCondition status = fileFormat.loadFile(path.c_str());
    if (status.bad()) {
        throw DicomFileReadError("Failed to parse DICOM file " + path + ": " + status.text());
    }

    DcmDataset* dataset = fileFormat.getDataset();
    if (dataset == nullptr) {
        throw DicomFileReadError("DICOM file has no dataset: " + path);
    }

    const std::string sopInstanceUid    = requireString(*dataset, DCM_SOPInstanceUID,    "SOP Instance UID");
    const std::string sopClassUid       = requireString(*dataset, DCM_SOPClassUID,       "SOP Class UID");
    const std::string studyInstanceUid  = requireString(*dataset, DCM_StudyInstanceUID,  "Study Instance UID");
    const std::string seriesInstanceUid = requireString(*dataset, DCM_SeriesInstanceUID, "Series Instance UID");
    const std::string modality          = requireString(*dataset, DCM_Modality,          "Modality");
    const std::string patientId         = optionalString(*dataset, DCM_PatientID);

    try {
        domain::DicomFile file{path, static_cast<std::uint64_t>(size)};
        domain::DicomInstanceMetadata metadata{
            sopInstanceUid,
            sopClassUid,
            studyInstanceUid,
            seriesInstanceUid,
            patientId,
            modality
        };
        return DicomFileReadResult{std::move(file), std::move(metadata)};
    } catch (const std::invalid_argument& e) {
        throw DicomFileReadError(
            std::string{"DICOM dataset failed domain validation: "} + e.what());
    }
}

}  // namespace vna::dicom
