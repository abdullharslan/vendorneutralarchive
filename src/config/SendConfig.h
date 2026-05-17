#pragma once

#include <cstdint>
#include <optional>
#include <string>

namespace vna::config {

struct SendConfig {
    std::optional<std::string> filePath;
    std::string host;
    std::uint16_t port = 0;
    std::string calledAe;
    std::string callingAe;
    std::uint32_t timeoutSeconds = 30;
    bool echoOnly = false;
};

struct SendCliResult {
    bool helpRequested = false;
    SendConfig config;
};

SendCliResult parseSendCli(int argc, char** argv);

std::string sendUsage(const char* programName);

}  // namespace vna::config
