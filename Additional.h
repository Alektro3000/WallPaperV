
#include "Base.h"
#include <iostream>
#include <comdef.h>
#include <Wbemidl.h>
#include <mmdeviceapi.h> 
#include <endpointvolume.h>
#include <audioclient.h>
#include <winuser.h>

namespace Additional {
    void UpdateMonitorCount();
    int GetMonitorCount();
    bool GetVolumeLevel(float& OutVolume);
    bool IsFullscreen();
}