#include <iostream>
#include <fstream>
#include <cereal/types/string.hpp>
#include <cereal/types/unordered_map.hpp>
#include <cereal/archives/binary.hpp>

#include "repository.h"
#include "cryptopp/sha.h"
#include "cryptopp/hex.h"
#include "cryptopp/filters.h"   // StringSource, StringSink, HashFilter
#include "cryptopp/files.h"     // FileSink
#include "cryptopp/zlib.h"

namespace crypto = CryptoPP;

void Repository::init() {
    if (fs::is_directory(DATA_DIR)) {
        std::cout << "The directory " << DATA_DIR << " already exists\n";
        return;
    }

    try {
        if (!fs::create_directory(DATA_DIR)) {
            std::cout << "Failed to create " << DATA_DIR << '\n';
            return;
        }
        if (!fs::create_directory(OBJECTS_DIR)) {
            std::cout << "Failed to create " << OBJECTS_DIR << '\n';
            return;
        }
    } catch (const fs::filesystem_error &e) {
        std::cout << "Caught exception: " << e.what() << '\n';
        return;
    }

    std::ofstream os { DATA_DIR / HEAD_FILE };
    os << "ref: refs/heads/" << DEFAULT_INIT_BRANCH;
}

void Repository::readFromIndex() {
    if (fs::exists(INDEX_FILE)) {
        std::ifstream is {INDEX_FILE, std::ios::binary};
        cereal::BinaryInputArchive iarchive {is};
        iarchive(stagingArea);
    } else {
        std::ofstream os {INDEX_FILE, std::ios::binary};
        os.close();
    }
}

void Repository::writeToIndex() {
    std::ofstream os {INDEX_FILE, std::ios::binary};
    cereal::BinaryOutputArchive oarchive {os};
    oarchive(stagingArea);
}

std::string Repository::hashFile(const std::string &filename) {
    std::string fileHashCode;
    crypto::SHA1 hash;
    std::ifstream is { filename, std::ios::binary };
    crypto::FileSource fs {
        is,
        true,
        new crypto::HashFilter {
            hash,
            new crypto::HexEncoder {
                new crypto::StringSink {
                    fileHashCode
                }
            }
        }
    };
    return fileHashCode;
}

void Repository::add(const std::string &filename) {
    if (!fs::exists(filename)) {
        std::cout << "The file " << filename << " doesn't exist\n";
        return;
    }

    std::string fileHashCode { hashFile(filename) };

    readFromIndex();

    auto it = stagingArea.find(filename);
    if (it != stagingArea.cend()) {
        if (fileHashCode == it->second.headHash) {
            if (it->second.state == FileState::STAGED) {
                it->second.state = FileState::UNMODIFIED;
            }
            else if (it->second.state == FileState::REMOVED) {
                it->second.state = FileState::UNMODIFIED;
            }
        } else {
            if (fileHashCode == it->second.stagingHash) return;

            it->second.stagingHash = fileHashCode;
            it->second.state = FileState::STAGED;
        }
    } else {
        StagingAreaEntry entry;
        entry.stagingHash = fileHashCode;
        entry.state = FileState::STAGED;
        it = stagingArea.insert({filename, entry}).first;
    }

    writeToIndex();

    if (it->second.state == FileState::STAGED) {
        fs::path dir { OBJECTS_DIR / fileHashCode.substr(0, 2) };
        fs::path compressedFile { dir / fileHashCode.substr(2) };
        if (!fs::exists(dir)) {
            fs::create_directory(dir);
        }
        fs::copy_file(filename, compressedFile);
    }
}