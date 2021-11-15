//check that libs are linked.
//#include <Logi/Logi.h>
//#include <M3D/M3D.h>
//#include <Tent/Tent.h>

#include <cstdlib>
#include <iostream>
#include <stdexcept>

#include "src/FirstApp.h"

int main() {
    cy3d::FirstApp app{};

    try
    {
        app.run();
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << "\n";
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
    return 0;
}