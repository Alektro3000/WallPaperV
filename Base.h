#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

const uint32_t PARTICLE_COUNT = 5120 + 1024;
const uint32_t PARTICLE_COUNT_Stable = 5120 + 1024 + 678;

static const uint32_t TextWidth = 2100;
static const float TextWidthNormalized = 2100/1080;

HWND GetWallpaper();
HWND GetWallpaperHandler();

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam);
void get_wallpaper_window();