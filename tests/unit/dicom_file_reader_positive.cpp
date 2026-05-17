#include "dicom/DicomFileReadError.h"
#include "dicom/DicomFileReader.h"
#include "domain/DicomFile.h"
#include "domain/DicomInstanceMetadata.h"

#include <exception>
#include <iostream>
#include <string>

namespace {

int failures = 0;

void check(bool condition, const std::string& label) {
    if (condition) {
        std::cout << "[pass] " << label << "\n";
    } else {
        std::cerr << "[fail] " << label << "\n";
        ++failures;
    }
}

}  // namespace

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Usage: dicom_file_reader_positive <fixture.dcm>\n";
        return 2;
    }
    const std::string fixturePath = argv[1];

    try {
        const vna::dicom::DicomFileReader reader;
        const auto result = reader.read(fixturePath);

        const auto& file = result.file;
        const auto& md = result.metadata;

        check(file.path() == fixturePath, "DicomFile.path() equals input path");
        check(file.sizeBytes() > 0,       "DicomFile.sizeBytes() is non-zero");

        check(md.sopInstanceUid()    == "1.2.3.4.5.6.7.8.9.0",
              "SOP Instance UID matches expected value");
        check(md.sopClassUid()       == "1.2.840.10008.5.1.4.1.1.7",
              "SOP Class UID matches expected value");
        check(md.studyInstanceUid()  == "1.2.3.4.5.6.7.8.9.1",
              "Study Instance UID matches expected value");
        check(md.seriesInstanceUid() == "1.2.3.4.5.6.7.8.9.2",
              "Series Instance UID matches expected value");

        check(md.patientId().empty(),  "Patient ID is empty (allowed)");
        check(!md.modality().empty(),  "Modality is non-empty");
        check(md.modality() == "SC",   "Modality value matches expected");
    } catch (const std::exception& e) {
        std::cerr << "[fail] unexpected exception: " << e.what() << "\n";
        return 1;
    }

    if (failures == 0) {
        std::cout << "All positive checks passed.\n";
        return 0;
    }
    std::cerr << failures << " check(s) failed.\n";
    return 1;
}
