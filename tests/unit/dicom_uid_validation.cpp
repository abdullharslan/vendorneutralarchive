#include "domain/DicomUid.h"

#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {

int failures = 0;

void expectValid(const std::string& uid, const std::string& label) {
    if (vna::domain::isValidDicomUid(uid)) {
        std::cout << "[pass] " << label << ": \"" << uid << "\"\n";
    } else {
        std::cerr << "[fail] " << label << ": \"" << uid
                  << "\" was rejected but should be valid\n";
        ++failures;
    }
}

void expectInvalid(const std::string& uid, const std::string& label) {
    if (!vna::domain::isValidDicomUid(uid)) {
        std::cout << "[pass] " << label << ": \"" << uid << "\"\n";
    } else {
        std::cerr << "[fail] " << label << ": \"" << uid
                  << "\" was accepted but should be invalid\n";
        ++failures;
    }
}

void expectRequireThrows(const std::string& uid, const std::string& label) {
    try {
        vna::domain::requireValidDicomUid(uid, "test UID");
    } catch (const std::invalid_argument& e) {
        std::cout << "[pass] " << label << ": " << e.what() << "\n";
        return;
    } catch (const std::exception& e) {
        std::cerr << "[fail] " << label
                  << ": expected invalid_argument, got: " << e.what() << "\n";
        ++failures;
        return;
    }
    std::cerr << "[fail] " << label
              << ": expected invalid_argument, nothing thrown for \""
              << uid << "\"\n";
    ++failures;
}

}  // namespace

int main() {
    expectValid("0",                              "single zero digit");
    expectValid("1.2.3.4",                        "short numeric UID");
    expectValid("1.2.840.10008.5.1.4.1.1.7",      "Secondary Capture SOP Class");
    expectValid("1.2.840.10008.1.1",              "Verification SOP Class");
    expectValid("0.0.0",                          "all-zero components");
    expectValid("1.2.3.4.5.6.7.8.9.0",            "fixture SOP Instance UID");
    expectValid(std::string(64, '1'),             "max-length all-ones UID");

    expectInvalid("",                             "empty UID");
    expectInvalid(std::string(65, '1'),           "UID longer than 64 characters");
    expectInvalid(".1.2",                         "leading dot");
    expectInvalid("1.2.",                         "trailing dot");
    expectInvalid("1..2",                         "consecutive dots");
    expectInvalid("01.2",                         "leading zero in first component");
    expectInvalid("1.02",                         "leading zero in middle component");
    expectInvalid("1.2.03",                       "leading zero in last component");
    expectInvalid("1.2.a",                        "non-digit character (letter)");
    expectInvalid("1.2.3 ",                       "trailing space");
    expectInvalid("../escape",                    "path traversal attempt");
    expectInvalid("1/2/3",                        "forward-slash UID");

    expectRequireThrows("",      "requireValidDicomUid throws on empty");
    expectRequireThrows("1..2",  "requireValidDicomUid throws on consecutive dots");

    return failures == 0 ? 0 : 1;
}
