#include "file_sys.h"
#include <algorithm>
#include <fstream>

namespace fs = std::filesystem;

std::vector<Entry> get_directory_contents(const fs::path& p, bool show_hidden) {
    std::vector<Entry> entries;
    if (!fs::exists(p) || !fs::is_directory(p)) {
        return entries;
    }
    
    // Add ".." if it has a parent
    if (p.has_parent_path() && p != p.parent_path()) {
        entries.push_back({"..", true, p.parent_path()});
    }

    try {
        for (const auto& entry : fs::directory_iterator(p)) {
            std::string filename = entry.path().filename().string();
            if (!show_hidden && !filename.empty() && filename[0] == '.') {
                continue;
            }
            bool is_dir = fs::is_directory(entry.status());
            entries.push_back({filename, is_dir, entry.path()});
        }
    } catch (...) {
        // Ignore permission denied etc.
    }

    // Sort: directories first, then alphabetical
    std::sort(entries.begin(), entries.end(), [](const Entry& a, const Entry& b) {
        if (a.name == "..") return true;
        if (b.name == "..") return false;
        if (a.is_dir != b.is_dir) return a.is_dir;
        return a.name < b.name;
    });

    return entries;
}

std::vector<std::string> get_file_preview(const fs::path& p, int max_lines) {
    std::vector<std::string> lines;
    
    std::error_code ec;
    if (!fs::is_regular_file(p, ec)) {
        return lines;
    }

    std::ifstream file(p, std::ios::binary);
    if (!file.is_open()) {
        return {"[Permission denied]"};
    }

    // Check if binary (heuristic: look for \0 in first 512 bytes)
    char buffer[512];
    file.read(buffer, sizeof(buffer));
    std::streamsize bytes_read = file.gcount();
    for (int i = 0; i < bytes_read; ++i) {
        if (buffer[i] == '\0') {
            return {"[Binary file]"};
        }
    }

    // Reopen as text
    file.close();
    file.open(p);
    
    std::string line;
    while (lines.size() < static_cast<size_t>(max_lines) && std::getline(file, line)) {
        for (char& c : line) {
            if (c == '\t') c = ' ';
            else if ((unsigned char)c < 32) c = ' '; // replace control chars with space
        }
        lines.push_back(line);
    }
    return lines;
}
