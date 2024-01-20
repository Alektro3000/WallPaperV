#pragma once
#include "MainApp.h"

#ifdef _CONSOLE
int main()
#else
int wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
#endif
{
    WallpaperApplication app;

    static_assert(sizeof(int) == 4);

    try {
        app.run();
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}