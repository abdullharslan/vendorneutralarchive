#pragma once

#include "domain/DicomStatus.h"

#include <string>

namespace vna::domain {

class StoreResult {
public:
    StoreResult(std::string sopInstanceUid, DicomStatus status, std::string message = {});

    const std::string& sopInstanceUid() const noexcept;
    const DicomStatus& status() const noexcept;
    const std::string& message() const noexcept;

    bool succeeded() const noexcept;

    friend bool operator==(const StoreResult&, const StoreResult&) noexcept;
    friend bool operator!=(const StoreResult&, const StoreResult&) noexcept;

private:
    std::string sopInstanceUid_;
    DicomStatus status_;
    std::string message_;
};

}  // namespace vna::domain
