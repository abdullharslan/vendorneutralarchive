#include "config/ConfigError.h"
#include "config/ReceiverConfig.h"
#include "dicom/StoreScp.h"
#include "dicom/StoreScpError.h"
#include "storage/LocalInstanceStorage.h"
#include "storage/StorageError.h"

#include <csignal>
#include <exception>
#include <iostream>
#include <string>

namespace {

vna::dicom::StoreScp* g_scp = nullptr;

extern "C" void handleStopSignal(int /*signum*/) {
    if (g_scp != nullptr) {
        g_scp->requestShutdown();
    }
}

}  // namespace

int main(int argc, char** argv) {
    using namespace vna::config;

    const char* programName = (argc > 0 && argv[0] != nullptr)
                                  ? argv[0] : "vna-receiver";

    std::cout.setf(std::ios::unitbuf);
    std::cerr.setf(std::ios::unitbuf);

    try {
        const auto result = parseReceiverCli(argc, argv);
        if (result.helpRequested) {
            std::cout << receiverUsage(programName);
            return 0;
        }

        const auto& cfg = result.config;
        std::cout << "vna-receiver: starting\n"
                  << "  host:          " << cfg.host
                  << " (informational; DCMTK acceptor binds to all interfaces)\n"
                  << "  port:          " << cfg.port << "\n"
                  << "  ae-title:      " << cfg.aeTitle << "\n"
                  << "  storage-path:  " << cfg.storagePath << "\n";

        vna::storage::LocalInstanceStorage storage{cfg.storagePath};

        vna::dicom::StoreScpConfig scpConfig{};
        scpConfig.aeTitle = cfg.aeTitle;
        scpConfig.host = cfg.host;
        scpConfig.port = cfg.port;
        scpConfig.timeoutSeconds = 30;

        vna::dicom::StoreScp scp{scpConfig, storage};
        scp.setLogCallback([](const std::string& message) {
            std::cout << "vna-receiver: " << message << "\n";
        });

        g_scp = &scp;
        std::signal(SIGINT, handleStopSignal);
        std::signal(SIGTERM, handleStopSignal);

        scp.run();
        g_scp = nullptr;
        return 0;
    } catch (const ConfigError& e) {
        std::cerr << programName << ": " << e.what() << "\n";
        std::cerr << "Run with --help for usage.\n";
        return 2;
    } catch (const vna::storage::StorageError& e) {
        std::cerr << programName << ": storage error: " << e.what() << "\n";
        return 1;
    } catch (const vna::dicom::StoreScpError& e) {
        std::cerr << programName << ": " << e.what() << "\n";
        return 1;
    } catch (const std::exception& e) {
        std::cerr << programName << ": unexpected error: " << e.what() << "\n";
        return 1;
    }
}
