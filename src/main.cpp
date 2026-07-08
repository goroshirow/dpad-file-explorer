#include "app.h"
#include <string>
#include <exception>

int main(int argc, char* argv[]) {
    bool show_hidden = false;
    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "--all") {
            show_hidden = true;
        }
    }

    try {
        App app(show_hidden);
        return app.run();
    } catch (const std::exception& e) {
        return 1;
    }
}
