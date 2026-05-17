#include "config/ConfigError.h"
#include "config/SendConfig.h"
#include "dicom/EchoScu.h"
#include "dicom/EchoScuError.h"
#include "domain/DicomNode.h"
#include "domain/EchoResult.h"

#include <iostream>
#include <stdexcept>

namespace {

int runEchoOnly(const vna::config::SendConfig& cfg, const char* programName) {
    std::cout << "vna-send: C-ECHO to " << cfg.calledAe << "@" << cfg.host
              << ":" << cfg.port << " (timeout " << cfg.timeoutSeconds << "s)\n";
    try {
        const vna::domain::DicomNode peer{cfg.calledAe, cfg.host, cfg.port};
        const vna::dicom::EchoScu scu;
        const auto result = scu.echo(cfg.callingAe, peer, cfg.timeoutSeconds);
        if (result.succeeded()) {
            std::cout << "vna-send: " << result.message() << "\n";
            return 0;
        }
        std::cerr << programName << ": " << result.message() << "\n";
        return 1;
    } catch (const vna::dicom::EchoScuError& e) {
        std::cerr << programName << ": " << e.what() << "\n";
        return 1;
    } catch (const std::invalid_argument& e) {
        std::cerr << programName << ": " << e.what() << "\n";
        return 1;
    }
}

int runNonEcho(const vna::config::SendConfig& cfg) {
    std::cout << "vna-send: parsed configuration\n";
    std::cout << "  host:        " << cfg.host << "\n";
    std::cout << "  port:        " << cfg.port << "\n";
    std::cout << "  called-ae:   " << cfg.calledAe << "\n";
    std::cout << "  calling-ae:  " << cfg.callingAe << "\n";
    std::cout << "  timeout:     " << cfg.timeoutSeconds << "s\n";
    std::cout << "  echo-only:   false\n";
    if (cfg.filePath) {
        std::cout << "  file:        " << *cfg.filePath << "\n";
    }
    std::cout << "C-STORE is not implemented yet.\n";
    return 1;
}

}  // namespace

int main(int argc, char** argv) {
    using namespace vna::config;

    const char* programName = (argc > 0 && argv[0] != nullptr) ? argv[0] : "vna-send";

    try {
        const auto result = parseSendCli(argc, argv);
        if (result.helpRequested) {
            std::cout << sendUsage(programName);
            return 0;
        }
        const auto& cfg = result.config;
        return cfg.echoOnly ? runEchoOnly(cfg, programName) : runNonEcho(cfg);
    } catch (const ConfigError& e) {
        std::cerr << programName << ": " << e.what() << "\n";
        std::cerr << "Run with --help for usage.\n";
        return 2;
    }
}
