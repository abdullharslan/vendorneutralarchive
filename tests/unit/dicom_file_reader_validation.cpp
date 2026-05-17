#include "dicom/DicomFileReadError.h"
#include "dicom/DicomFileReader.h"

#include <exception>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <string>

namespace {

bool expectReadError(const std::function<void()>& fn, const std::string& label) {
    try {
        fn();
    } catch (const vna::dicom::DicomFileReadError& e) {
        std::cout << "[pass] " << label << ": " << e.what() << "\n";
        return true;
    } catch (const std::exception& e) {
        std::cerr << "[fail] " << label
                  << ": unexpected exception type: " << e.what() << "\n";
        return false;
    }
    std::cerr << "[fail] " << label
              << ": expected DicomFileReadError, none thrown\n";
    return false;
}

}  // namespace

int main() {
    namespace fs = std::filesystem;

    const vna::dicom::DicomFileReader reader;
    bool ok = true;

    ok &= expectReadError([&]() {
        reader.read("/this/path/does/not/exist/vna-test-12345.dcm");
    }, "nonexistent file");

    const fs::path emptyPath = fs::temp_directory_path() / "vna-empty-file.dcm";
    fs::remove(emptyPath);
    { std::ofstream f{emptyPath, std::ios::binary | std::ios::trunc}; }
    ok &= expectReadError([&]() {
        reader.read(emptyPath.string());
    }, "empty file");
    fs::remove(emptyPath);

    const fs::path invalidPath = fs::temp_directory_path() / "vna-not-dicom.dcm";
    fs::remove(invalidPath);
    {
        std::ofstream f{invalidPath, std::ios::binary | std::ios::trunc};
        const char junk[] = "This is not a DICOM Part 10 file.";
        f.write(junk, sizeof(junk) - 1);
    }
    ok &= expectReadError([&]() {
        reader.read(invalidPath.string());
    }, "invalid DICOM file");
    fs::remove(invalidPath);

    return ok ? 0 : 1;
}
