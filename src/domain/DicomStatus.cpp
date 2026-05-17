#include "domain/DicomStatus.h"

namespace vna::domain {

DicomStatus::DicomStatus(std::uint16_t code) noexcept : code_(code) {}

DicomStatus DicomStatus::success() noexcept { return DicomStatus{0x0000}; }
DicomStatus DicomStatus::cancel() noexcept { return DicomStatus{0xFE00}; }
DicomStatus DicomStatus::pending() noexcept { return DicomStatus{0xFF00}; }

std::uint16_t DicomStatus::code() const noexcept { return code_; }

DicomStatusCategory DicomStatus::category() const noexcept {
    if (code_ == 0x0000) {
        return DicomStatusCategory::Success;
    }
    if (code_ == 0xFE00) {
        return DicomStatusCategory::Cancel;
    }
    if ((code_ & 0xFF00) == 0xFF00) {
        return DicomStatusCategory::Pending;
    }
    if ((code_ & 0xF000) == 0xB000) {
        return DicomStatusCategory::Warning;
    }
    return DicomStatusCategory::Failure;
}

bool DicomStatus::isSuccess() const noexcept { return category() == DicomStatusCategory::Success; }
bool DicomStatus::isWarning() const noexcept { return category() == DicomStatusCategory::Warning; }
bool DicomStatus::isFailure() const noexcept { return category() == DicomStatusCategory::Failure; }
bool DicomStatus::isCancel() const noexcept  { return category() == DicomStatusCategory::Cancel;  }
bool DicomStatus::isPending() const noexcept { return category() == DicomStatusCategory::Pending; }

bool operator==(const DicomStatus& a, const DicomStatus& b) noexcept { return a.code_ == b.code_; }
bool operator!=(const DicomStatus& a, const DicomStatus& b) noexcept { return !(a == b); }

std::string toString(DicomStatusCategory category) {
    switch (category) {
        case DicomStatusCategory::Success: return "Success";
        case DicomStatusCategory::Warning: return "Warning";
        case DicomStatusCategory::Failure: return "Failure";
        case DicomStatusCategory::Cancel:  return "Cancel";
        case DicomStatusCategory::Pending: return "Pending";
    }
    return "Unknown";
}

}  // namespace vna::domain
