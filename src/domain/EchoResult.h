#pragma once

#include "domain/DicomStatus.h"

#include <string>

namespace vna::domain {

class EchoResult {
public:
    EchoResult(DicomStatus status, std::string message = {});

    const DicomStatus& status() const noexcept;
    const std::string& message() const noexcept;

    bool succeeded() const noexcept;

    friend bool operator==(const EchoResult&, const EchoResult&) noexcept;
    friend bool operator!=(const EchoResult&, const EchoResult&) noexcept;

private:
    DicomStatus status_;
    std::string message_;
};

}  // namespace vna::domain
