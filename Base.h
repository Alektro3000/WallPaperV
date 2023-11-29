#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

const uint32_t PARTICLE_COUNT = 5120 + 678 + 1024;

const uint32_t WIDTH = 1920;
const uint32_t HEIGHT = 1080;

static const uint32_t TextWidth = 2100;

HWND GetWallpaper();
HWND GetWallpaperHandler();

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam);
void get_wallpaper_window();