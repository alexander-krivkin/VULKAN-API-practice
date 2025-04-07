#include <iostream>
#include "Application.h"


int main() {
    ak::Application app;

    try {
        app.run();
    }
    catch (const std::exception& ex) {
        std::cerr << ex.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}