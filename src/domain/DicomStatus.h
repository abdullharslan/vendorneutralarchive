#pragma once

#include <cstdint>
#include <string>

namespace vna::domain {

enum class DicomStatusCategory {
    Success,
    Warning,
    Failure,
    Cancel,
    Pending,
};

class DicomStatus {
public:
    explicit DicomStatus(std::uint16_t code) noexcept;

    static DicomStatus success() noexcept;
    static DicomStatus cancel() noexcept;
    static DicomStatus pending() noexcept;

    std::uint16_t code() const noexcept;
    DicomStatusCategory category() const noexcept;

    bool isSuccess() const noexcept;
    bool isWarning() const noexcept;
    bool isFailure() const noexcept;
    bool isCancel() const noexcept;
    bool isPending() const noexcept;

    friend bool operator==(const DicomStatus&, const DicomStatus&) noexcept;
    friend bool operator!=(const DicomStatus&, const DicomStatus&) noexcept;

private:
    std::uint16_t code_;
};

std::string toString(DicomStatusCategory category);

}  // namespace vna::domain
