#include "domain/StoreResult.h"

#include <stdexcept>
#include <utility>

namespace vna::domain {

StoreResult::StoreResult(std::string sopInstanceUid, DicomStatus status, std::string message)
    : sopInstanceUid_(std::move(sopInstanceUid)), status_(status), message_(std::move(message)) {
    if (sopInstanceUid_.empty()) {
        throw std::invalid_argument("StoreResult SOP Instance UID must not be empty");
    }
}

const std::string& StoreResult::sopInstanceUid() const noexcept { return sopInstanceUid_; }
const DicomStatus& StoreResult::status() const noexcept { return status_; }
const std::string& StoreResult::message() const noexcept { return message_; }
bool StoreResult::succeeded() const noexcept { return status_.isSuccess(); }

bool operator==(const StoreResult& a, const StoreResult& b) noexcept {
    return a.sopInstanceUid_ == b.sopInstanceUid_
        && a.status_          == b.status_
        && a.message_         == b.message_;
}
bool operator!=(const StoreResult& a, const StoreResult& b) noexcept { return !(a == b); }

}  // namespace vna::domain
