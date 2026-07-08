# D-pad File Explorer

**English** | [日本語](./README-ja.md)

A lightning-fast, keyboard-driven (Arrow keys) file explorer for the Linux terminal. 

Instead of traditional file managers that require hitting `Enter` to open directories and typing commands to go back, D-pad File Explorer utilizes a 4-column "Miller columns" layout. It allows you to effortlessly navigate your entire filesystem using only the arrow keys, dramatically speeding up directory traversal.

## Features

- **Intuitive Arrow-Key Navigation**: Use `Up`/`Down` to select, `Right` to enter, and `Left` to go back.
- **Miller Columns Layout**: Displays Grandparent, Parent, Current, and Child directories side-by-side. Always know exactly where you are and where you can go.
- **Dynamic Resizing**: Automatically adapts the number of files shown and column widths based on your terminal window size.
- **Shell Integration (`cd`)**: Press `Enter` on a directory to immediately exit the explorer and `cd` your terminal into that path.
- **Quick File Open**: Press `Enter` on a file to view and edit its contents directly in the terminal using `vi`, ensuring full cross-environment compatibility.
- **Instant File Preview**: If the focused item is a file, the 4th column intelligently displays a live preview of its contents (safely skipping binary or unreadable files).
- **Hidden Files**: Full support for displaying hidden files and directories.

## Requirements

- **OS**: Linux / macOS (Any environment with a standard POSIX terminal)
- **Compiler**: `g++` (C++17 or higher)
- **Build Tool**: `cmake` (version 3.10 or higher), `make`
- **Library**: `ncurses` (Wide character support)

### Installing Dependencies (Ubuntu/Debian)

```bash
sudo apt update
sudo apt install -y g++ cmake make libncurses5-dev libncursesw5-dev
```

## Installation & Build

Clone the repository and build using `cmake` and `make`:

```bash
git clone https://github.com/goroshirow/dpad-file-explorer.git
cd dpad-file-explorer
mkdir build
cd build
cmake ..
make
```

This will generate the `dpad_explorer` executable in the `build` directory.

## Initial Setup (Crucial for `cd` functionality)

To enable the core feature—pressing `Enter` on a directory to actually change your current shell directory—you **must** use a shell wrapper function. A child process (the C++ program) cannot change the parent shell's working directory on its own.

By defining this function, you can also call the explorer from anywhere using the `dpad` command, without needing to place the executable in your `bin` directory.

Run the following command to append the function to your `~/.bashrc` (or `~/.zshrc`).
(Make sure to replace `/path/to/...` with the actual absolute path on your system before running it)

```bash
cat << 'EOF' >> ~/.bashrc
function dpad() {
    # Specify the absolute path to the compiled executable
    # Example: local dest=$(/home/user/dpad-file-explorer/build/dpad_explorer)
    local dest=$(/path/to/dpad-file-explorer/build/dpad_explorer)
    
    if [ -n "$dest" ]; then
        cd "$dest"
    fi
}
EOF
```

After adding it, reload your shell config:

```bash
source ~/.bashrc
```

Now, simply typing `dpad` in your terminal will launch the explorer from anywhere, and exit to the selected directory.

### Command Line Arguments

| Argument | Description |
| --- | --- |
| `--all` | Show hidden files and directories (files starting with a dot). |

## Keybindings and Commands

| Key | Action |
| --- | --- |
| `↑` (Up) | Move selection up in the current directory |
| `↓` (Down) | Move selection down in the current directory |
| `→` (Right) | **On Directory:** Enter the selected directory.<br>**On File:** Focus the Preview column to scroll its contents. |
| `←` (Left) | Go back to the parent directory (or exit Preview focus). |
| `Enter` | **On Directory:** Exit explorer and `cd` into it.<br>**On File:** View/edit file contents with `vi`. |
| `/` (Slash) | **Search Mode:** Start typing to incrementally search the current directory. |
| `:` (Colon) | **Command Mode:** Type a command and execute it (see below). |
| `ESC` | Exit Search Mode, Command Mode, or Preview focus. |
| `q` or `Q` | Quit the explorer without changing directories |

### Command Mode (`:`)

Press `:`, type one of the following commands, and hit `Enter`:

| Command | Description |
| --- | --- |
| `R` or `r` | **Rename** the focused item. |
| `D` or `d` | **Delete** the focused item. Prompts for confirmation on non-empty directories. |
| `md` | **Make Directory** (create a new directory) in the current directory. |
| `mf` | **Make File** (create a new file) in the current directory. |
| `unzip` | **Unzip** the focused zip file into the current directory. |

## License

[MIT License](./LICENSE)
