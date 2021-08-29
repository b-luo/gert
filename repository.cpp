#include <iostream>
#include <fstream>

#include "repository.h"

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