#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

constexpr uint32_t PARTICLE_COUNT = 1024 + 512 + (5120) + 1024;
constexpr uint32_t PARTICLE_COUNT_Stable = PARTICLE_COUNT + 678*2;

constexpr uint32_t TextWidth = 2100;
constexpr float TextWidthNormalized = 2100/1080;

HWND GetWallpaper();
HWND GetWallpaperHandler();

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam);
void get_wallpaper_window();