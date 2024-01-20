
#include "Base.h"
#include <iostream>
#include <comdef.h>
#include <Wbemidl.h>
#include <mmdeviceapi.h> 
#include <endpointvolume.h>
#include <audioclient.h>
#include <winuser.h>

int GetMonitorCount();
namespace Usage {

    bool GetVolumeLevel(float& OutVolume);
    bool IsFullscreen();
}