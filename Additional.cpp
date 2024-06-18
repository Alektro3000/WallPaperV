#include "Additional.h"


namespace Additional {

    int CachedMonitorCount;
    std::vector<RECT> CachedMonitorPositions;

    BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData)
    {
        int* Count = (int*)dwData;
        (*Count)++;
        return TRUE;
    }
    BOOL CALLBACK MonitorEnumProcPos(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData)
    {
        ((std::vector<RECT>*)dwData)->push_back(*lprcMonitor);
        return TRUE;
    }

    void UpdateMonitors()
    {
        int Count = 0;
        std::vector<RECT> Pos = {};
        if (EnumDisplayMonitors(NULL, NULL, MonitorEnumProc, (LPARAM)&Count))
        {
            EnumDisplayMonitors(NULL, NULL, MonitorEnumProcPos, (LPARAM)&Pos);
            CachedMonitorPositions = Pos;
            CachedMonitorCount = Count;
        }
        else
            CachedMonitorCount = -1;
    }

    int GetMonitorCount()
    {
        if (CachedMonitorCount == -1)
            UpdateMonitors();
        return CachedMonitorCount;
    }
    RECT GetMonitorBoundingBox()
    {
        if (CachedMonitorCount == -1)
            UpdateMonitors();
        RECT Out = {};
        for (auto rect : CachedMonitorPositions)
        {
            UnionRect(&Out, &Out, &rect);
        }
        
        return Out;
    }
    std::vector<RECT> GetMonitorsBoxes()
    {
        if (CachedMonitorCount == -1)
            UpdateMonitors();
        return CachedMonitorPositions;
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