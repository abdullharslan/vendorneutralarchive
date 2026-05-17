#include "config/ReceiverConfig.h"

#include "config/CliParsing.h"
#include "config/ConfigError.h"
#include "domain/AeTitle.h"

#include <stdexcept>
#include <string>
#include <string_view>

namespace vna::config {

ReceiverCliResult parseReceiverCli(int argc, char** argv) {
    ReceiverCliResult result{};
    ReceiverConfig& cfg = result.config;

    bool hostSet = false;
    bool portSet = false;
    bool aeSet = false;
    bool storageSet = false;

    for (int i = 1; i < argc; ++i) {
        const std::string_view arg{argv[i]};

        if (arg == "--help") {
            result.helpRequested = true;
            return result;
        } else if (arg == "--host") {
            cfg.host = internal::takeValue(i, argc, argv, arg);
            hostSet = true;
        } else if (arg == "--port") {
            cfg.port = internal::parsePort(internal::takeValue(i, argc, argv, arg), arg);
            portSet = true;
        } else if (arg == "--ae-title") {
            cfg.aeTitle = internal::takeValue(i, argc, argv, arg);
            aeSet = true;
        } else if (arg == "--storage-path") {
            cfg.storagePath = internal::takeValue(i, argc, argv, arg);
            storageSet = true;
        } else {
            throw ConfigError(std::string{"unknown option: "} + std::string{arg});
        }
    }

    if (!hostSet)    throw ConfigError("--host is required");
    if (!portSet)    throw ConfigError("--port is required");
    if (!aeSet)      throw ConfigError("--ae-title is required");
    if (!storageSet) throw ConfigError("--storage-path is required");

    if (cfg.host.empty())        throw ConfigError("--host must not be empty");
    if (cfg.storagePath.empty()) throw ConfigError("--storage-path must not be empty");

    try {
        domain::requireValidAeTitle(cfg.aeTitle, "--ae-title");
    } catch (const std::invalid_argument& e) {
        throw ConfigError(e.what());
    }

    return result;
}

std::string receiverUsage(const char* programName) {
    const std::string name = (programName && *programName) ? programName : "vna-receiver";
    return "Usage: " + name + " [options]\n"
           "\n"
           "Listen for DICOM associations and accept C-STORE requests.\n"
           "(DICOM networking is not yet implemented in this milestone.)\n"
           "\n"
           "Required:\n"
           "  --host <host>             Bind hostname or IP address\n"
           "  --port <port>             Listening port (1-65535)\n"
           "  --ae-title <ae-title>     Local AE title (1-16 chars)\n"
           "  --storage-path <path>     Filesystem path for received instances\n"
           "\n"
           "Optional:\n"
           "  --help                    Print this help and exit\n";
}

}  // namespace vna::config
