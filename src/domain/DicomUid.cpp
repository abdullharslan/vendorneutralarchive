#include "domain/DicomUid.h"

#include <stdexcept>
#include <string>

namespace vna::domain {

namespace {

constexpr std::size_t kMaxUidLength = 64;

}  // namespace

bool isValidDicomUid(const std::string& uid) noexcept {
    if (uid.empty() || uid.size() > kMaxUidLength) {
        return false;
    }
    if (uid.front() == '.' || uid.back() == '.') {
        return false;
    }

    std::size_t componentStart = 0;
    for (std::size_t i = 0; i <= uid.size(); ++i) {
        const bool atEnd = (i == uid.size());
        if (atEnd || uid[i] == '.') {
            if (i == componentStart) {
                return false;  // empty component (leading, trailing, or "..").
            }
            const std::size_t componentLen = i - componentStart;
            if (componentLen > 1 && uid[componentStart] == '0') {
                return false;  // leading zero in a multi-digit component.
            }
            componentStart = i + 1;
            continue;
        }
        if (uid[i] < '0' || uid[i] > '9') {
            return false;
        }
    }
    return true;
}

void requireValidDicomUid(const std::string& uid, const char* context) {
    if (!isValidDicomUid(uid)) {
        throw std::invalid_argument(
            std::string{context} + " is not a valid DICOM UID: \"" + uid + "\"");
    }
}

}  // namespace vna::domain
