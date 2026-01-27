#pragma once

#include <filesystem>
#include <vector>
namespace fs = std::filesystem;

// helper for deleting a file after destructing a object pf this struct
class FileDeleter {
public:
    explicit FileDeleter(const fs::path& p) : p(p) {}
    ~FileDeleter();

private:
    fs::path p;
};

bool is_zip_ready(const fs::path& dir);
fs::path get_home_path();
fs::path findFolder(const std::vector<fs::path>& locations, const std::string& folderName);
bool zipDir(fs::path& sourceDir, fs::path& outputZip, bool verbose);
