#include "MainApp.h"
#include "Parser.h"

int main()
{
    Parser a;
    a.init();
    WallpaperApplication app;


    try {
        app.run();
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}