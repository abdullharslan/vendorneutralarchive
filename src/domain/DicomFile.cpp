#include "domain/DicomFile.h"

#include <stdexcept>
#include <utility>

namespace vna::domain {

DicomFile::DicomFile(std::string path, std::uint64_t sizeBytes)
    : path_(std::move(path)), sizeBytes_(sizeBytes) {
    if (path_.empty()) {
        throw std::invalid_argument("DicomFile path must not be empty");
    }
    if (sizeBytes_ == 0) {
        throw std::invalid_argument("DicomFile size must be greater than zero bytes");
    }
}

const std::string& DicomFile::path() const noexcept { return path_; }
std::uint64_t DicomFile::sizeBytes() const noexcept { return sizeBytes_; }

bool operator==(const DicomFile& a, const DicomFile& b) noexcept {
    return a.path_ == b.path_ && a.sizeBytes_ == b.sizeBytes_;
}
bool operator!=(const DicomFile& a, const DicomFile& b) noexcept { return !(a == b); }

}  // namespace vna::domain
