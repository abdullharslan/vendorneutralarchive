#pragma once

#include "storage/InstanceStorage.h"

#include <atomic>
#include <cstdint>
#include <functional>
#include <string>

namespace vna::dicom {

struct StoreScpConfig {
    std::string aeTitle;
    std::string host;
    std::uint16_t port = 0;
    std::uint32_t timeoutSeconds = 30;
};

class StoreScp {
public:
    using LogCallback = std::function<void(const std::string&)>;

    StoreScp(StoreScpConfig config, storage::InstanceStorage& storage);

    // Installs a callback invoked for each human-readable progress and result
    // message. The default is a no-op. Passing nullptr/empty restores the no-op.
    void setLogCallback(LogCallback callback);

    // Blocks: opens a listening socket on the configured port and serves
    // incoming associations sequentially until requestShutdown() is called.
    // Throws StoreScpError if the network cannot be initialized.
    void run();

    // Asks the run loop to exit before accepting the next association.
    // Safe to call from a signal handler on platforms where std::atomic<bool>
    // is lock-free (e.g. macOS and Linux on x86_64/aarch64).
    void requestShutdown() noexcept;

private:
    StoreScpConfig config_;
    storage::InstanceStorage* storage_;
    LogCallback log_;
    std::atomic<bool> shutdown_{false};
};

}  // namespace vna::dicom
