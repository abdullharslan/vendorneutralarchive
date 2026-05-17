#include "domain/AeTitle.h"

#include <stdexcept>
#include <string>

namespace vna::domain {

bool isValidAeTitle(const std::string& title) noexcept {
    if (title.empty() || title.size() > 16) {
        return false;
    }
    for (char c : title) {
        const auto u = static_cast<unsigned char>(c);
        if (u < 0x20 || u > 0x7E) {
            return false;
        }
        if (c == '\\') {
            return false;
        }
    }
    return true;
}

void requireValidAeTitle(const std::string& title, const char* context) {
    if (title.empty()) {
        throw std::invalid_argument(std::string{context} + " must not be empty");
    }
    if (title.size() > 16) {
        throw std::invalid_argument(std::string{context} + " must be at most 16 characters");
    }
    for (char c : title) {
        const auto u = static_cast<unsigned char>(c);
        if (u < 0x20 || u > 0x7E) {
            throw std::invalid_argument(std::string{context} + " contains control characters");
        }
        if (c == '\\') {
            throw std::invalid_argument(std::string{context} + " must not contain backslash");
        }
    }
}

}  // namespace vna::domain
