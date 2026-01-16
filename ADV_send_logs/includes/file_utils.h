#pragma once

#include "file_utils.h"
#include <filesystem>
#include <vector>
namespace fs = std::filesystem;

// helper for deleting a file after destructing a object pf this struct
struct FileDeleter {
    fs::path p;
    ~FileDeleter();
};

fs::path get_home_path();
fs::path findFolder(const std::vector<fs::path>& locations, const std::string& folderName);
bool zipDir(std::string& sourceDir, std::string& outputZip);
