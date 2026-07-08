#ifndef APP_H
#define APP_H

#include <string>
#include <vector>
#include <filesystem>
#include "file_sys.h"

class App {
public:
    App();
    ~App();
    int run();

private:
    void draw();
    void handle_input();
    void draw_col(int col, const std::vector<Entry>& entries, int sel_idx, const std::filesystem::path& highlight_path, bool is_active);

    std::filesystem::path current_path;
    std::vector<Entry> current_entries;
    int selected_idx;
    std::string cd_path;

    bool search_mode;
    std::string search_query;

    std::filesystem::path last_preview_path;
    std::vector<std::string> cached_preview;
    int preview_selected_idx;
    bool preview_focused;

    bool command_mode;
    std::string command_input;
    bool rename_mode;
    std::string rename_input;
    bool delete_confirm_mode;
    bool create_file_mode;
    bool create_dir_mode;
    std::string create_input;

    std::vector<Entry> parent_entries;
    std::vector<Entry> grandparent_entries;
    std::vector<Entry> child_entries;
    std::vector<std::string> file_preview;
    bool is_file_preview;

    int max_y, max_x;
    int col_width;
    int list_height;
    bool should_exit;
};

#endif // APP_H
