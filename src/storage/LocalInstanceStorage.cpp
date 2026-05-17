#include "storage/LocalInstanceStorage.h"

#include "storage/StorageError.h"

#include <cstdint>
#include <filesystem>
#include <string>
#include <system_error>
#include <utility>

namespace vna::storage {

namespace {

bool containsUnsafePathChars(const std::string& uid) {
    if (uid.find('/')  != std::string::npos) return true;
    if (uid.find('\\') != std::string::npos) return true;
    if (uid.find("..") != std::string::npos) return true;
    return false;
}

}  // namespace

LocalInstanceStorage::LocalInstanceStorage(std::string rootPath)
    : rootPath_(std::move(rootPath)) {
    if (rootPath_.empty()) {
        throw StorageError("storage root path must not be empty");
    }

    namespace fs = std::filesystem;
    std::error_code ec;
    fs::create_directories(rootPath_, ec);
    if (ec) {
        throw StorageError("failed to create storage root '" + rootPath_ +
                           "': " + ec.message());
    }
    if (!fs::is_directory(rootPath_, ec) || ec) {
        throw StorageError("storage root '" + rootPath_ + "' is not a directory");
    }
}

const std::string& LocalInstanceStorage::rootPath() const noexcept {
    return rootPath_;
}

std::string LocalInstanceStorage::prepareDestination(const std::string& sopInstanceUid) {
    if (sopInstanceUid.empty()) {
        throw StorageError("SOP Instance UID must not be empty");
    }
    if (containsUnsafePathChars(sopInstanceUid)) {
        throw StorageError("SOP Instance UID contains unsafe path characters: " +
                           sopInstanceUid);
    }

    namespace fs = std::filesystem;
    const fs::path target = fs::path{rootPath_} / (sopInstanceUid + ".dcm");

    std::error_code ec;
    fs::create_directories(target.parent_path(), ec);
    if (ec) {
        throw StorageError("failed to prepare storage directory '" +
                           target.parent_path().string() + "': " + ec.message());
    }
    return target.string();
}

bool LocalInstanceStorage::isPersisted(const std::string& destinationPath) const {
    namespace fs = std::filesystem;
    std::error_code ec;
    if (!fs::exists(destinationPath, ec) || ec) return false;
    if (!fs::is_regular_file(destinationPath, ec) || ec) return false;
    const std::uintmax_t size = fs::file_size(destinationPath, ec);
    if (ec) return false;
    return size > 0;
}

}  // namespace vna::storage
