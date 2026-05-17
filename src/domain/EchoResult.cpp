#include "domain/EchoResult.h"

#include <utility>

namespace vna::domain {

EchoResult::EchoResult(DicomStatus status, std::string message)
    : status_(status), message_(std::move(message)) {}

const DicomStatus& EchoResult::status() const noexcept { return status_; }
const std::string& EchoResult::message() const noexcept { return message_; }
bool EchoResult::succeeded() const noexcept { return status_.isSuccess(); }

bool operator==(const EchoResult& a, const EchoResult& b) noexcept {
    return a.status_ == b.status_ && a.message_ == b.message_;
}
bool operator!=(const EchoResult& a, const EchoResult& b) noexcept { return !(a == b); }

}  // namespace vna::domain
