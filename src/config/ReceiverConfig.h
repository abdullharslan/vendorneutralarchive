#pragma once

#include <cstdint>
#include <string>

namespace vna::config {

struct ReceiverConfig {
    std::string host;
    std::uint16_t port = 0;
    std::string aeTitle;
    std::string storagePath;
};

struct ReceiverCliResult {
    bool helpRequested = false;
    ReceiverConfig config;
};

ReceiverCliResult parseReceiverCli(int argc, char** argv);

std::string receiverUsage(const char* programName);

}  // namespace vna::config
