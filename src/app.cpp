#include "app.h"
#include "utils.h"
#include <ncurses.h>
#include <iostream>
#include <algorithm>
#include <fstream>
#include <cstdlib>

namespace fs = std::filesystem;

App::App(bool show_hidden) : 
    selected_idx(0), cd_path(""), search_mode(false), search_query(""),
    preview_selected_idx(0), preview_focused(false),
    command_mode(false), command_input(""), rename_mode(false), rename_input(""),
    delete_confirm_mode(false), create_file_mode(false), create_dir_mode(false), create_input(""),
    is_file_preview(false), show_hidden(show_hidden), should_exit(false)
{
    setenv("ESCDELAY", "25", 1); // Make ESC key responsive
    FILE* tty = fopen("/dev/tty", "r+");
    if (!tty) {
        std::cerr << "Failed to open /dev/tty" << std::endl;
        exit(1);
    }
    SCREEN* term = newterm(NULL, tty, tty);
    set_term(term);
    
    noecho();
    cbreak();
    keypad(stdscr, TRUE);
    curs_set(0);
    start_color();
    use_default_colors();
    init_pair(1, COLOR_CYAN, -1); // Directory color
    init_pair(2, COLOR_BLACK, COLOR_WHITE); // Selected item color
    init_pair(3, COLOR_WHITE, COLOR_BLUE); // Selected Directory color

    current_path = fs::current_path();
    current_entries = get_directory_contents(current_path, show_hidden);
    selected_idx = (current_entries.size() > 1 && current_entries[0].name == "..") ? 1 : 0;
}

App::~App() {
    endwin();
    // FILE* and SCREEN* leak but that's what was in main.cpp
    if (!cd_path.empty()) {
        std::cout << cd_path << std::endl;
    }
}

int App::run() {
    while (!should_exit) {
        getmaxyx(stdscr, max_y, max_x);
        col_width = max_x / 4;
        list_height = max_y - 2;

        fs::path parent_path = current_path.parent_path();
        fs::path grandparent_path = parent_path.parent_path();

        if (current_path != parent_path) {
            parent_entries = get_directory_contents(parent_path, show_hidden);
        } else {
            parent_entries.clear();
        }
        
        if (parent_path != grandparent_path) {
            grandparent_entries = get_directory_contents(grandparent_path, show_hidden);
        } else {
            grandparent_entries.clear();
        }

        child_entries.clear();
        file_preview.clear();
        is_file_preview = false;

        if (!current_entries.empty() && selected_idx < static_cast<int>(current_entries.size())) {
            if (current_entries[selected_idx].is_dir) {
                if (current_entries[selected_idx].name != "..") {
                    child_entries = get_directory_contents(current_entries[selected_idx].path, show_hidden);
                }
            } else {
                is_file_preview = true;
                if (current_entries[selected_idx].path != last_preview_path) {
                    last_preview_path = current_entries[selected_idx].path;
                    cached_preview = get_file_preview(last_preview_path, 10000);
                    preview_selected_idx = 0;
                }
                file_preview = cached_preview;
            }
        }

        draw();
        handle_input();
    }
    return 0;
}

void App::draw_col(int col, const std::vector<Entry>& entries, int sel_idx, const fs::path& highlight_path, bool is_active) {
    int start_x = col * col_width;
    
    int effective_sel_idx = sel_idx;
    if (effective_sel_idx == -1 && !highlight_path.empty()) {
        for (int i = 0; i < static_cast<int>(entries.size()); ++i) {
            if (entries[i].path == highlight_path) {
                effective_sel_idx = i;
                break;
            }
        }
    }

    int offset = 0;
    if (effective_sel_idx >= list_height / 2) {
        offset = effective_sel_idx - list_height / 2;
    }
    if (entries.size() > 0 && offset > static_cast<int>(entries.size()) - list_height) {
        offset = std::max(0, static_cast<int>(entries.size()) - list_height);
    }

    for (int i = 0; i < list_height && (i + offset) < static_cast<int>(entries.size()); ++i) {
        const auto& entry = entries[i + offset];
        bool is_selected = (i + offset) == sel_idx && is_active;
        bool is_highlighted = (!is_active && entry.path == highlight_path);

        if (is_selected) {
            attron(COLOR_PAIR(2));
        } else if (is_highlighted) {
            attron(COLOR_PAIR(3));
        } else if (entry.is_dir) {
            attron(COLOR_PAIR(1));
        }

        std::string disp = truncate_str(entry.name, col_width - 1);
        mvprintw(i, start_x, "%s", disp.c_str());

        if (is_selected || is_highlighted) {
            for(int s = disp.length(); s < col_width - 1; ++s) {
                printw(" ");
            }
        }

        if (is_selected) attroff(COLOR_PAIR(2));
        else if (is_highlighted) attroff(COLOR_PAIR(3));
        else if (entry.is_dir) attroff(COLOR_PAIR(1));
    }
}

void App::draw() {
    clear();

    std::string original_name;
    if (rename_mode && !current_entries.empty() && selected_idx < static_cast<int>(current_entries.size())) {
        original_name = current_entries[selected_idx].name;
        current_entries[selected_idx].name = rename_input + "_";
    }

    draw_col(0, grandparent_entries, -1, current_path.parent_path(), false);
    draw_col(1, parent_entries, -1, current_path, false);
    draw_col(2, current_entries, selected_idx, "", !preview_focused);
    
    if (rename_mode && !original_name.empty()) {
        current_entries[selected_idx].name = original_name;
    }
    
    if (is_file_preview) {
        int start_x = 3 * col_width;
        
        int p_offset = 0;
        if (preview_selected_idx >= list_height / 2) {
            p_offset = preview_selected_idx - list_height / 2;
        }
        if (file_preview.size() > 0 && p_offset > static_cast<int>(file_preview.size()) - list_height) {
            p_offset = std::max(0, static_cast<int>(file_preview.size()) - list_height);
        }

        for (int i = 0; i < list_height && (i + p_offset) < static_cast<int>(file_preview.size()); ++i) {
            bool is_p_selected = preview_focused && ((i + p_offset) == preview_selected_idx);
            if (is_p_selected) attron(COLOR_PAIR(2));
            
            std::string disp = truncate_str(file_preview[i + p_offset], col_width - 1);
            mvprintw(i, start_x, "%s", disp.c_str());
            
            if (is_p_selected) {
                for(int s = disp.length(); s < col_width - 1; ++s) printw(" ");
                attroff(COLOR_PAIR(2));
            }
        }
    } else {
        draw_col(3, child_entries, -1, "", false);
    }

    attron(A_REVERSE);
    std::string status_line = " ";
    if (search_mode) {
        status_line += "/" + search_query;
    } else if (rename_mode) {
        status_line += "-- RENAME --";
    } else if (delete_confirm_mode) {
        std::error_code ec;
        if (fs::is_directory(current_entries[selected_idx].path, ec)) {
            status_line += "Directory not empty. Delete '" + current_entries[selected_idx].name + "' and its contents? (y/n)";
        } else {
            status_line += "File not empty. Delete '" + current_entries[selected_idx].name + "'? (y/n)";
        }
    } else if (create_file_mode) {
        status_line += "New file: " + create_input + "_";
    } else if (create_dir_mode) {
        status_line += "New directory: " + create_input + "_";
    } else if (command_mode) {
        status_line += ":" + command_input + "_";
    } else if (preview_focused) {
        status_line += current_entries[selected_idx].name + " +" + std::to_string(preview_selected_idx + 1);
    } else {
        if (!current_entries.empty() && selected_idx < static_cast<int>(current_entries.size())) {
            status_line += current_entries[selected_idx].path.string();
        } else {
            status_line += current_path.string();
        }
    }
    status_line = truncate_str(status_line, max_x - 1);
    mvprintw(max_y - 1, 0, "%s", status_line.c_str());
    for(int s = status_line.length(); s < max_x; ++s) printw(" ");
    attroff(A_REVERSE);

    refresh();
}

void App::handle_input() {
    int ch = getch();

    if (preview_focused) {
        if (ch == KEY_LEFT || ch == 'q' || ch == 27) {
            preview_focused = false;
        } else if (ch == KEY_UP) {
            if (preview_selected_idx > 0) preview_selected_idx--;
        } else if (ch == KEY_DOWN) {
            if (preview_selected_idx < static_cast<int>(file_preview.size()) - 1) {
                preview_selected_idx++;
            }
        }
        return;
    }

    if (rename_mode) {
        if (ch == 27) {
            rename_mode = false;
        } else if (ch == '\n' || ch == '\r' || ch == KEY_ENTER) {
            if (!rename_input.empty() && rename_input != current_entries[selected_idx].name) {
                fs::path new_path = current_path / rename_input;
                std::error_code ec;
                fs::rename(current_entries[selected_idx].path, new_path, ec);
                current_entries = get_directory_contents(current_path, show_hidden);
                for (int i = 0; i < static_cast<int>(current_entries.size()); ++i) {
                    if (current_entries[i].name == rename_input) {
                        selected_idx = i;
                        break;
                    }
                }
            }
            rename_mode = false;
        } else if (ch == KEY_BACKSPACE || ch == 127 || ch == 8) {
            if (!rename_input.empty()) rename_input.pop_back();
        } else if (isprint(ch)) {
            rename_input += static_cast<char>(ch);
        }
        return;
    }

    if (delete_confirm_mode) {
        if (ch == 'y' || ch == 'Y') {
            std::error_code ec;
            fs::remove_all(current_entries[selected_idx].path, ec);
            current_entries = get_directory_contents(current_path, show_hidden);
            if (selected_idx >= static_cast<int>(current_entries.size())) {
                selected_idx = std::max(0, static_cast<int>(current_entries.size()) - 1);
            }
        }
        delete_confirm_mode = false;
        return;
    }

    if (create_file_mode || create_dir_mode) {
        if (ch == 27) {
            create_file_mode = false;
            create_dir_mode = false;
        } else if (ch == '\n' || ch == '\r' || ch == KEY_ENTER) {
            if (!create_input.empty()) {
                fs::path new_path = current_path / create_input;
                std::error_code ec;
                if (create_dir_mode) {
                    fs::create_directory(new_path, ec);
                } else {
                    if (!fs::exists(new_path)) {
                        std::ofstream outfile(new_path);
                        outfile.close();
                    }
                }
                current_entries = get_directory_contents(current_path, show_hidden);
                for (int i = 0; i < static_cast<int>(current_entries.size()); ++i) {
                    if (current_entries[i].name == create_input) {
                        selected_idx = i;
                        break;
                    }
                }
            }
            create_file_mode = false;
            create_dir_mode = false;
        } else if (ch == KEY_BACKSPACE || ch == 127 || ch == 8) {
            if (!create_input.empty()) create_input.pop_back();
        } else if (isprint(ch)) {
            create_input += static_cast<char>(ch);
        }
        return;
    }

    if (command_mode) {
        if (ch == 27) {
            command_mode = false;
            command_input = "";
        } else if (ch == KEY_BACKSPACE || ch == 127 || ch == 8) {
            if (!command_input.empty()) command_input.pop_back();
            if (command_input.empty()) command_mode = false;
        } else if (isprint(ch)) {
            command_input += static_cast<char>(ch);
            
            if (command_input == "q" || command_input == "Q") {
                should_exit = true;
                return;
            } else if (command_input == "r" || command_input == "R") {
                command_mode = false;
                if (!current_entries.empty() && current_entries[selected_idx].name != "..") {
                    rename_mode = true;
                    rename_input = current_entries[selected_idx].name;
                }
            } else if (command_input == "d" || command_input == "D") {
                command_mode = false;
                if (!current_entries.empty() && current_entries[selected_idx].name != "..") {
                    std::error_code ec;
                    fs::path p = current_entries[selected_idx].path;
                    bool is_dir = fs::is_directory(p, ec);
                    bool is_file = fs::is_regular_file(p, ec);
                    bool can_delete_immediately = false;
                    if (is_dir && fs::is_empty(p, ec)) {
                        can_delete_immediately = true;
                    } else if (is_file && fs::file_size(p, ec) == 0) {
                        can_delete_immediately = true;
                    }
                    if (can_delete_immediately) {
                        fs::remove(p, ec);
                        current_entries = get_directory_contents(current_path, show_hidden);
                        if (selected_idx >= static_cast<int>(current_entries.size())) {
                            selected_idx = std::max(0, static_cast<int>(current_entries.size()) - 1);
                        }
                    } else {
                        delete_confirm_mode = true;
                    }
                }
            } else if (command_input == "mf") {
                command_mode = false;
                create_file_mode = true;
                create_input = "";
            } else if (command_input == "md") {
                command_mode = false;
                create_dir_mode = true;
                create_input = "";
            } else if (command_input == "unzip") {
                command_mode = false;
                if (!current_entries.empty() && current_entries[selected_idx].name != ".." && !current_entries[selected_idx].is_dir) {
                    endwin();
                    std::string cmd = "unzip \"" + current_entries[selected_idx].path.string() + "\" -d \"" + current_path.string() + "\" < /dev/tty > /dev/tty";
                    int ret = system(cmd.c_str());
                    (void)ret;
                    refresh();
                    current_entries = get_directory_contents(current_path, show_hidden);
                }
            } else {
                bool valid_prefix = false;
                std::vector<std::string> valid_commands = {"r", "R", "d", "D", "mf", "md", "unzip", "q", "Q"};
                for (const auto& cmd : valid_commands) {
                    if (cmd.length() >= command_input.length() && cmd.substr(0, command_input.length()) == command_input) {
                        valid_prefix = true;
                        break;
                    }
                }
                if (!valid_prefix) {
                    command_mode = false;
                }
            }
        }
        return;
    }

    if (search_mode) {
        if (ch == 27) {
            search_mode = false;
            search_query = "";
            return;
        } else if (ch == KEY_LEFT || ch == KEY_RIGHT || ch == KEY_UP || ch == KEY_DOWN || ch == '\n' || ch == '\r' || ch == KEY_ENTER) {
            search_mode = false;
            search_query = "";
        } else if (ch == KEY_BACKSPACE || ch == 127 || ch == 8) {
            if (!search_query.empty()) {
                search_query.pop_back();
            }
        } else if (isprint(ch)) {
            search_query += static_cast<char>(ch);
        } else {
            return;
        }

        if (search_mode) {
            if (!search_query.empty()) {
                for (int i = 0; i < static_cast<int>(current_entries.size()); ++i) {
                    auto it = std::search(
                        current_entries[i].name.begin(), current_entries[i].name.end(),
                        search_query.begin(), search_query.end(),
                        [](char ch1, char ch2) { return std::tolower(ch1) == std::tolower(ch2); }
                    );
                    if (it != current_entries[i].name.end()) {
                        selected_idx = i;
                        break;
                    }
                }
            }
            return;
        }
    }

    if (ch == ':') {
        command_mode = true;
        command_input = "";
    } else if (ch == KEY_UP) {
        if (selected_idx > 0) selected_idx--;
    } else if (ch == KEY_DOWN) {
        if (selected_idx < (int)current_entries.size() - 1) selected_idx++;
    } else if (ch == KEY_LEFT) {
        if (current_path != current_path.parent_path()) {
            fs::path old_path = current_path;
            current_path = current_path.parent_path();
            current_entries = get_directory_contents(current_path, show_hidden);
            selected_idx = (current_entries.size() > 1 && current_entries[0].name == "..") ? 1 : 0;
            for (int i = 0; i < static_cast<int>(current_entries.size()); ++i) {
                if (current_entries[i].path == old_path) {
                    selected_idx = i;
                    break;
                }
            }
        }
    } else if (ch == KEY_RIGHT) {
        if (!current_entries.empty()) {
            if (current_entries[selected_idx].is_dir) {
                if (current_entries[selected_idx].name == "..") {
                    fs::path old_path = current_path;
                    current_path = current_path.parent_path();
                    current_entries = get_directory_contents(current_path, show_hidden);
                    selected_idx = (current_entries.size() > 1 && current_entries[0].name == "..") ? 1 : 0;
                    for (int i = 0; i < static_cast<int>(current_entries.size()); ++i) {
                        if (current_entries[i].path == old_path) {
                            selected_idx = i;
                            break;
                        }
                    }
                } else {
                    current_path = current_entries[selected_idx].path;
                    current_entries = get_directory_contents(current_path, show_hidden);
                    selected_idx = (current_entries.size() > 1 && current_entries[0].name == "..") ? 1 : 0;
                }
            } else if (is_file_preview) {
                preview_focused = true;
            }
        }
     } else if (ch == '\n' || ch == '\r' || ch == KEY_ENTER) {
          if (!current_entries.empty()) {
              if (current_entries[selected_idx].is_dir) {
                  if (current_entries[selected_idx].name == "..") {
                      cd_path = current_path.parent_path().string();
                  } else {
                      cd_path = current_entries[selected_idx].path.string();
                  }
                  should_exit = true;
              } else {
                  endwin();
                  std::string cmd = "vi \"" + current_entries[selected_idx].path.string() + "\" < /dev/tty > /dev/tty";
                  int ret = system(cmd.c_str());
                  (void)ret;
                  refresh();
              }
          }
     } else if (isprint(ch)) {
        search_mode = true;
        search_query = static_cast<char>(ch);
        if (!search_query.empty()) {
            for (int i = 0; i < static_cast<int>(current_entries.size()); ++i) {
                auto it = std::search(
                    current_entries[i].name.begin(), current_entries[i].name.end(),
                    search_query.begin(), search_query.end(),
                    [](char ch1, char ch2) { return std::tolower(ch1) == std::tolower(ch2); }
                );
                if (it != current_entries[i].name.end()) {
                    selected_idx = i;
                    break;
                }
            }
        }
    }
}
