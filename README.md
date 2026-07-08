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
- **Library**: `ncurses` (Wide character support)

### Installing Dependencies (Ubuntu/Debian)

```bash
sudo apt update
sudo apt install -y g++ make libncurses5-dev libncursesw5-dev
```

## Installation & Build

Clone the repository and run `make`:

```bash
git clone https://github.com/yourusername/dpad-file-explorer.git
cd dpad-file-explorer
make
```

This will generate the `dpad_explorer` executable.

## Initial Setup (Crucial for `cd` functionality)

To enable the core feature—pressing `Enter` on a directory to actually change your current shell directory—you **must** use a shell wrapper function. A child process (the C++ program) cannot change the parent shell's working directory on its own.

Add the following to your `~/.bashrc` (or `~/.zshrc`):

```bash
function dpad() {
    # Specify the absolute path to the compiled executable
    # Example: local dest=$(/home/user/dpad-file-explorer/dpad_explorer)
    local dest=$(/path/to/dpad_explorer)
    
    if [ -n "$dest" ]; then
        cd "$dest"
    fi
}
```

After adding it, reload your shell config:

```bash
source ~/.bashrc
```

## Keybindings

| Key | Action |
| --- | --- |
| `↑` (Up) | Move selection up in the current directory |
| `↓` (Down) | Move selection down in the current directory |
| `→` (Right) | **On Directory:** Enter the selected directory.<br>**On File:** Focus the Preview column to scroll its contents. |
| `←` (Left) | Go back to the parent directory (or exit Preview focus). |
| `Enter` | **On Directory:** Exit explorer and `cd` into it.<br>**On File:** View/edit file contents with `vi`. |
| `/` (Slash) | **Search Mode:** Start typing to incrementally search the current directory. |
| `:` (Colon) | **Command Mode:** Type `R` to Rename, or `D` to Delete the focused item. |
| `ESC` | Exit Search Mode or Preview focus. |
| `q` or `Q` | Quit the explorer without changing directories |

## License

[MIT License](./LICENSE)
