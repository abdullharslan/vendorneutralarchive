#include "config/ConfigError.h"
#include "config/SendConfig.h"

#include <exception>
#include <iostream>
#include <string>
#include <vector>

namespace {

int failures = 0;

void expectConfigError(std::vector<std::string> args,
                       const std::string& label) {
    std::vector<char*> argv;
    argv.reserve(args.size());
    for (auto& a : args) {
        argv.push_back(a.data());
    }
    try {
        vna::config::parseSendCli(static_cast<int>(argv.size()), argv.data());
    } catch (const vna::config::ConfigError& e) {
        std::cout << "[pass] " << label << ": " << e.what() << "\n";
        return;
    } catch (const std::exception& e) {
        std::cerr << "[fail] " << label
                  << ": expected ConfigError, got other exception: "
                  << e.what() << "\n";
        ++failures;
        return;
    }
    std::cerr << "[fail] " << label
              << ": expected ConfigError, none thrown\n";
    ++failures;
}

}  // namespace

int main() {
    expectConfigError(
        {"vna-send", "--echo-only", "--host", "127.0.0.1", "--port", "11112",
         "--called-ae", "TEST_SCP", "--calling-ae", "VNA",
         "--timeout", "4000000000"},
        "huge --timeout above int::max rejected at CLI");

    expectConfigError(
        {"vna-send", "--echo-only", "--host", "127.0.0.1", "--port", "11112",
         "--called-ae", "TEST_SCP", "--calling-ae", "VNA",
         "--timeout", "9999999999"},
        "out-of-range --timeout above uint32::max rejected at CLI");

    return failures == 0 ? 0 : 1;
}
