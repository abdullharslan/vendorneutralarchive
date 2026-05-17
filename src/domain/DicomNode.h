#pragma once

#include <cstdint>
#include <string>

namespace vna::domain {

class DicomNode {
public:
    DicomNode(std::string aeTitle, std::string host, std::uint16_t port);

    const std::string& aeTitle() const noexcept;
    const std::string& host() const noexcept;
    std::uint16_t port() const noexcept;

    friend bool operator==(const DicomNode&, const DicomNode&) noexcept;
    friend bool operator!=(const DicomNode&, const DicomNode&) noexcept;

private:
    std::string aeTitle_;
    std::string host_;
    std::uint16_t port_;
};

}  // namespace vna::domain
