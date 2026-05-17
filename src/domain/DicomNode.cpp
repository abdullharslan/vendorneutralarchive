#include "domain/DicomNode.h"

#include "domain/AeTitle.h"

#include <stdexcept>
#include <utility>

namespace vna::domain {

namespace {

void validateHost(const std::string& host) {
    if (host.empty()) {
        throw std::invalid_argument("DicomNode host must not be empty");
    }
}

void validatePort(std::uint16_t port) {
    if (port == 0) {
        throw std::invalid_argument("DicomNode port must be in the range 1-65535");
    }
}

}  // namespace

DicomNode::DicomNode(std::string aeTitle, std::string host, std::uint16_t port)
    : aeTitle_(std::move(aeTitle)), host_(std::move(host)), port_(port) {
    requireValidAeTitle(aeTitle_, "DicomNode AE title");
    validateHost(host_);
    validatePort(port_);
}

const std::string& DicomNode::aeTitle() const noexcept { return aeTitle_; }
const std::string& DicomNode::host() const noexcept { return host_; }
std::uint16_t DicomNode::port() const noexcept { return port_; }

bool operator==(const DicomNode& a, const DicomNode& b) noexcept {
    return a.aeTitle_ == b.aeTitle_ && a.host_ == b.host_ && a.port_ == b.port_;
}
bool operator!=(const DicomNode& a, const DicomNode& b) noexcept { return !(a == b); }

}  // namespace vna::domain
