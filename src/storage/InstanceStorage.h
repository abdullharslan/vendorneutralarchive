#pragma once

#include <string>

namespace vna::storage {

class InstanceStorage {
public:
    virtual ~InstanceStorage() = default;

    // Returns the filesystem path where a Part 10 DICOM file for the given
    // SOP Instance UID is to be written, creating any missing parent
    // directories. Throws StorageError on filesystem failures or unsafe UIDs.
    virtual std::string prepareDestination(const std::string& sopInstanceUid) = 0;

    // Returns true if the given destination path contains a non-empty regular
    // file. Used after writing to verify persistence before reporting success.
    virtual bool isPersisted(const std::string& destinationPath) const = 0;
};

}  // namespace vna::storage
