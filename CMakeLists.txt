cmake_minimum_required(VERSION 3.10)
project(dpad_explorer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find ncurses library
find_package(Curses REQUIRED)

# Include directories
include_directories(${CURSES_INCLUDE_DIRS})

# Add executable
add_executable(dpad_explorer src/main.cpp src/app.cpp src/file_sys.cpp src/utils.cpp)

# Link libraries
target_link_libraries(dpad_explorer ${CURSES_LIBRARIES})
