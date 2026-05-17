#pragma once

#include "storage/InstanceStorage.h"

#include <string>

namespace vna::storage {

class LocalInstanceStorage : public InstanceStorage {
public:
    explicit LocalInstanceStorage(std::string rootPath);

    const std::string& rootPath() const noexcept;

    std::string prepareDestination(const std::string& sopInstanceUid) override;
    bool isPersisted(const std::string& destinationPath) const override;

private:
    std::string rootPath_;
};

}  // namespace vna::storage
