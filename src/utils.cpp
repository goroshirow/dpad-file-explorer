#include "utils.h"
#include <cstddef>

std::string truncate_str(const std::string& str, int width) {
    if (str.length() <= static_cast<size_t>(width)) return str;
    if (width <= 3) return str.substr(0, width);
    return str.substr(0, width - 3) + "...";
}
