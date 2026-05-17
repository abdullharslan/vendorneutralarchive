#include "storage/LocalInstanceStorage.h"
#include "storage/StorageError.h"

#include <exception>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <string>

namespace {

int failures = 0;

void check(bool condition, const std::string& label) {
    if (condition) {
        std::cout << "[pass] " << label << "\n";
    } else {
        std::cerr << "[fail] " << label << "\n";
        ++failures;
    }
}

void expectStorageError(const std::function<void()>& fn,
                        const std::string& label) {
    try {
        fn();
    } catch (const vna::storage::StorageError& e) {
        std::cout << "[pass] " << label << ": " << e.what() << "\n";
        return;
    } catch (const std::exception& e) {
        std::cerr << "[fail] " << label
                  << ": expected StorageError, got: " << e.what() << "\n";
        ++failures;
        return;
    }
    std::cerr << "[fail] " << label << ": expected StorageError, none thrown\n";
    ++failures;
}

}  // namespace

int main() {
    namespace fs = std::filesystem;

    expectStorageError([]() {
        vna::storage::LocalInstanceStorage s{""};
    }, "empty root path rejected");

    const fs::path tempRoot =
        fs::temp_directory_path() / "vna-local-instance-storage-test";
    fs::remove_all(tempRoot);

    {
        vna::storage::LocalInstanceStorage storage{tempRoot.string()};
        check(fs::is_directory(tempRoot), "root directory created on construction");
        check(storage.rootPath() == tempRoot.string(),
              "rootPath() returns the configured root");

        const std::string uid = "1.2.3.4.5";
        const std::string destination = storage.prepareDestination(uid);
        check(destination == (tempRoot / "1.2.3.4.5.dcm").string(),
              "destination is root/uid.dcm");

        check(!storage.isPersisted(destination),
              "isPersisted returns false before any write");

        {
            std::ofstream f{destination, std::ios::binary | std::ios::trunc};
            f << "non-empty bytes";
        }
        check(storage.isPersisted(destination),
              "isPersisted returns true for non-empty file");

        const fs::path emptyPath = tempRoot / "empty.dcm";
        { std::ofstream f{emptyPath, std::ios::binary | std::ios::trunc}; }
        check(!storage.isPersisted(emptyPath.string()),
              "isPersisted returns false for empty file");

        check(!storage.isPersisted(tempRoot.string()),
              "isPersisted returns false for a directory path");

        expectStorageError([&]() {
            storage.prepareDestination("");
        }, "empty SOP Instance UID rejected");

        expectStorageError([&]() {
            storage.prepareDestination("../escape");
        }, "UID with '..' rejected");

        expectStorageError([&]() {
            storage.prepareDestination("a/b");
        }, "UID with forward-slash rejected");

        expectStorageError([&]() {
            storage.prepareDestination("a\\b");
        }, "UID with backslash rejected");
    }

    fs::remove_all(tempRoot);

    return failures == 0 ? 0 : 1;
}
