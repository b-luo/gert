#ifndef REPOSITORY_H__
#define REPOSITORY_H__

#include <string>
#include <filesystem>
#include <unordered_map>

namespace fs = std::filesystem;

const fs::path WORK_TREE { fs::current_path() };
const fs::path DATA_DIR { WORK_TREE / ".gert" };
const fs::path HEAD_FILE { DATA_DIR / "HEAD" };
const std::string DEFAULT_INIT_BRANCH { "main" };
const fs::path OBJECTS_DIR { DATA_DIR / "objects" };
const fs::path INDEX_FILE { DATA_DIR / "index" };

enum class FileState { STAGED, UNSTAGED, REMOVED, UNTRACKED, UNMODIFIED };

struct StagingAreaEntry {
    std::string headHash;
    std::string stagingHash;
    FileState state;

    template<class Archive>
    void serialize(Archive &archive) {
        archive(headHash, stagingHash, state);
    }
};

class Repository {
    std::unordered_map<std::string, StagingAreaEntry> stagingArea;
    
    void readFromIndex();
    void writeToIndex();
    std::string hashFile(const std::string &filename);
public:
    void init();
    void add(const std::string &filename);
};

#endif