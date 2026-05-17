#include "config/ConfigError.h"
#include "config/ReceiverConfig.h"

#include <iostream>

int main(int argc, char** argv) {
    using namespace vna::config;

    const char* programName = (argc > 0 && argv[0] != nullptr) ? argv[0] : "vna-receiver";

    try {
        const auto result = parseReceiverCli(argc, argv);
        if (result.helpRequested) {
            std::cout << receiverUsage(programName);
            return 0;
        }

        const auto& cfg = result.config;
        std::cout << "vna-receiver: parsed configuration\n";
        std::cout << "  host:          " << cfg.host << "\n";
        std::cout << "  port:          " << cfg.port << "\n";
        std::cout << "  ae-title:      " << cfg.aeTitle << "\n";
        std::cout << "  storage-path:  " << cfg.storagePath << "\n";
        std::cout << "DICOM networking is not implemented yet.\n";
        return 0;
    } catch (const ConfigError& e) {
        std::cerr << programName << ": " << e.what() << "\n";
        std::cerr << "Run with --help for usage.\n";
        return 2;
    }
}
