
#include "Base.h"
#include <iostream>
#include <comdef.h>
#include <Wbemidl.h>
#include <mmdeviceapi.h> 
#include <endpointvolume.h>
#include <audioclient.h>
#include <winuser.h>
#include "vector"

namespace Additional {
    void UpdateMonitors();
    int GetMonitorCount();
    RECT GetMonitorBoundingBox();
    std::vector<RECT> GetMonitorsBoxes();
    bool GetVolumeLevel(float& OutVolume);
    bool IsFullscreen();
}