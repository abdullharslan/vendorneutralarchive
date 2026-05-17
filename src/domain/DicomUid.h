#pragma once

#include <string>

namespace vna::domain {

// DICOM UIDs (PS3.5 Section 9.1) are strings of digits and '.' separators, at
// most 64 characters, with no leading/trailing dot, no empty components, and
// no multi-digit component that starts with '0'. This validator enforces those
// rules; it is intentionally DCMTK-free so it can live in the domain layer.
bool isValidDicomUid(const std::string& uid) noexcept;

void requireValidDicomUid(const std::string& uid,
                          const char* context = "DICOM UID");

}  // namespace vna::domain
