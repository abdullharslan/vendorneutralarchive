#include "config/SendConfig.h"

#include "config/CliParsing.h"
#include "config/ConfigError.h"
#include "domain/AeTitle.h"

#include <limits>
#include <stdexcept>
#include <string>
#include <string_view>

namespace vna::config {

SendCliResult parseSendCli(int argc, char** argv) {
    SendCliResult result{};
    SendConfig& cfg = result.config;

    bool hostSet = false;
    bool portSet = false;
    bool calledAeSet = false;
    bool callingAeSet = false;

    for (int i = 1; i < argc; ++i) {
        const std::string_view arg{argv[i]};

        if (arg == "--help") {
            result.helpRequested = true;
            return result;
        } else if (arg == "--file") {
            cfg.filePath = internal::takeValue(i, argc, argv, arg);
        } else if (arg == "--host") {
            cfg.host = internal::takeValue(i, argc, argv, arg);
            hostSet = true;
        } else if (arg == "--port") {
            cfg.port = internal::parsePort(internal::takeValue(i, argc, argv, arg), arg);
            portSet = true;
        } else if (arg == "--called-ae") {
            cfg.calledAe = internal::takeValue(i, argc, argv, arg);
            calledAeSet = true;
        } else if (arg == "--calling-ae") {
            cfg.callingAe = internal::takeValue(i, argc, argv, arg);
            callingAeSet = true;
        } else if (arg == "--timeout") {
            const std::string raw = internal::takeValue(i, argc, argv, arg);
            const std::uint32_t parsed = internal::parsePositiveUint32(raw, arg);
            if (parsed >
                static_cast<std::uint32_t>(std::numeric_limits<int>::max())) {
                throw ConfigError(std::string{arg} +
                                  " must not exceed " +
                                  std::to_string(std::numeric_limits<int>::max()) +
                                  " seconds, got \"" + raw + "\"");
            }
            cfg.timeoutSeconds = parsed;
        } else if (arg == "--echo-only") {
            cfg.echoOnly = true;
        } else {
            throw ConfigError(std::string{"unknown option: "} + std::string{arg});
        }
    }

    if (!hostSet)      throw ConfigError("--host is required");
    if (!portSet)      throw ConfigError("--port is required");
    if (!calledAeSet)  throw ConfigError("--called-ae is required");
    if (!callingAeSet) throw ConfigError("--calling-ae is required");

    if (cfg.echoOnly && cfg.filePath) {
        throw ConfigError("--file must not be combined with --echo-only");
    }
    if (!cfg.echoOnly && !cfg.filePath) {
        throw ConfigError("--file is required unless --echo-only is set");
    }

    if (cfg.host.empty()) {
        throw ConfigError("--host must not be empty");
    }
    if (cfg.filePath && cfg.filePath->empty()) {
        throw ConfigError("--file must not be empty");
    }

    try {
        domain::requireValidAeTitle(cfg.calledAe, "--called-ae");
        domain::requireValidAeTitle(cfg.callingAe, "--calling-ae");
    } catch (const std::invalid_argument& e) {
        throw ConfigError(e.what());
    }

    return result;
}

std::string sendUsage(const char* programName) {
    const std::string name = (programName && *programName) ? programName : "vna-send";
    return "Usage: " + name + " [options]\n"
           "\n"
           "Send a DICOM file via C-STORE or test connectivity via C-ECHO.\n"
           "(DICOM networking is not yet implemented in this milestone.)\n"
           "\n"
           "Required:\n"
           "  --host <host>             Remote DICOM peer hostname or IP\n"
           "  --port <port>             Remote DICOM peer port (1-65535)\n"
           "  --called-ae <ae-title>    Peer AE title (1-16 chars)\n"
           "  --calling-ae <ae-title>   Local AE title (1-16 chars)\n"
           "  --file <path>             DICOM file to send (required unless --echo-only)\n"
           "\n"
           "Optional:\n"
           "  --timeout <seconds>       Network timeout in seconds (default 30)\n"
           "  --echo-only               Perform C-ECHO only, do not send a file\n"
           "  --help                    Print this help and exit\n";
}

}  // namespace vna::config
