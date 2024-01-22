#include "Additional.h"


namespace Additional {

    int CachedMonitorCount;

    BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData)
    {
        int* Count = (int*)dwData;
        (*Count)++;
        return TRUE;
    }

    void UpdateMonitorCount()
    {
        int Count = 0;
        if (EnumDisplayMonitors(NULL, NULL, MonitorEnumProc, (LPARAM)&Count))
            CachedMonitorCount = Count;
        else
            CachedMonitorCount = -1;
    }

    int GetMonitorCount()
    {
        if (CachedMonitorCount == -1)
            UpdateMonitorCount();
        return CachedMonitorCount;
    }

    bool GetVolumeLevel(float& OutVolume)
    {
        HRESULT hr;
        IMMDeviceEnumerator* pDeviceEnumerator = 0;
        IMMDevice* pDevice = 0;
        IAudioEndpointVolume* pAudioEndpointVolume = 0;

        try {
            hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**)&pDeviceEnumerator);
            if (FAILED(hr)) throw "CoCreateInstance";
            hr = pDeviceEnumerator->GetDefaultAudioEndpoint(eRender, eMultimedia, &pDevice);
            if (FAILED(hr)) throw "GetDefaultAudioEndpoint";
            hr = pDevice->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_ALL, NULL, (void**)&pAudioEndpointVolume);
            if (FAILED(hr)) throw "pDevice->Active";

            hr = pAudioEndpointVolume->GetMasterVolumeLevelScalar(&OutVolume);
            if (FAILED(hr)) throw "SetMasterVolumeLevelScalar";

            pAudioEndpointVolume->Release();
            pDevice->Release();
            pDeviceEnumerator->Release();
            return true;
        }
        catch (...) {
            if (pAudioEndpointVolume) pAudioEndpointVolume->Release();
            if (pDevice) pDevice->Release();
            if (pDeviceEnumerator) pDeviceEnumerator->Release();
            throw;
        }
        return false;
    }

    bool IsFullscreen()
    {
        HWND hWnd = GetForegroundWindow();
        RECT appBounds;
        RECT background;
        GetWindowRect(GetDesktopWindow(), &background);
        if (hWnd != GetDesktopWindow() && hWnd != GetShellWindow())
        {
            if (hWnd != GetWallpaperHandler() && hWnd != GetWallpaper())
            {
                GetWindowRect(hWnd, &appBounds);

                if (appBounds.right == appBounds.left)
                    if (appBounds.top == appBounds.bottom)
                        return true;

                for (int i = 0; i < GetMonitorCount(); ++i)
                    if (appBounds.left <= background.right * i + background.left
                        && appBounds.right >= background.right * (i+1)
                        && appBounds.top <= background.top 
                        && appBounds.bottom >= background.bottom)
                        return true;
            }
        }
        return false;
    }
}