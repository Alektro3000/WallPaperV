#include "Base.h"

HWND wallpaper_hwnd = nullptr;

HWND wallpaper_handler = nullptr;

HWND GetWallpaper()
{
    if (wallpaper_hwnd == nullptr)
        get_wallpaper_window();    return wallpaper_hwnd;
}
HWND GetWallpaperHandler()
{
    if (wallpaper_handler == nullptr)
        get_wallpaper_window();    return wallpaper_handler;
}

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam)
{
    HWND p = FindWindowEx(hwnd, NULL, L"SHELLDLL_DefView", NULL);
    if (p)
    {
        wallpaper_handler = hwnd;
        wallpaper_hwnd = FindWindowEx(NULL, hwnd, L"WorkerW", NULL);
    }
    return true;
}

void get_wallpaper_window()
{
    HWND progman = FindWindow(L"ProgMan", NULL);
    SendMessageTimeout(progman, 0x052C, 0, 0, SMTO_NORMAL, 1000, nullptr);

    EnumWindows(EnumWindowsProc, NULL);
    return;

}