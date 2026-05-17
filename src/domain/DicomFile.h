#pragma once

#include <cstdint>
#include <string>

namespace vna::domain {

class DicomFile {
public:
    DicomFile(std::string path, std::uint64_t sizeBytes);

    const std::string& path() const noexcept;
    std::uint64_t sizeBytes() const noexcept;

    friend bool operator==(const DicomFile&, const DicomFile&) noexcept;
    friend bool operator!=(const DicomFile&, const DicomFile&) noexcept;

private:
    std::string path_;
    std::uint64_t sizeBytes_;
};

}  // namespace vna::domain
