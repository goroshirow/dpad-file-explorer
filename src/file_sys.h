#ifndef FILE_SYS_H
#define FILE_SYS_H

#include <string>
#include <vector>
#include <filesystem>

struct Entry {
    std::string name;
    bool is_dir;
    std::filesystem::path path;
};

std::vector<Entry> get_directory_contents(const std::filesystem::path& p, bool show_hidden);
std::vector<std::string> get_file_preview(const std::filesystem::path& p, int max_lines);

#endif // FILE_SYS_H
