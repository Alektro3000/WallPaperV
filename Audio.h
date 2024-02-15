#pragma once
#include <vector>
#include <Windows.h>
#include <Audioclient.h>
#include <mfidl.h>
#include <mfapi.h>
#include <mmdeviceapi.h>
#include <iostream>
#include <Mmreg.h>
#include <iostream>

#pragma comment(lib, "Mf.lib")
#pragma comment(lib, "Mfplat.lib")

#define REFTIMES_PER_SEC  10000000
#define REFTIMES_PER_MILLISEC  10000

#define EXIT_ON_ERROR(hres)  \
              if (FAILED(hres)) { goto Exit; }
#define SAFE_RELEASE(punk)  \
              if ((punk) != NULL)  \
                { (punk)->Release(); (punk) = NULL; }
class Audio
{
    REFERENCE_TIME hnsRequestedDuration = REFTIMES_PER_SEC;
    REFERENCE_TIME hnsActualDuration;
    UINT32 bufferFrameCount;
    UINT32 numFramesAvailable;
    IMMDeviceEnumerator* pEnumerator = NULL;
    IMMDevice* pDevice = NULL;
    IAudioClient* pAudioClient = NULL;
    IAudioCaptureClient* pCaptureClient = NULL;
    WAVEFORMATEXTENSIBLE* pwfx = NULL;


public:
	Audio();
	std::vector<double> GetSound(int discr = 100);


private:
    std::vector<double> out;
    std::vector<int> outWeight;
    Audio(const Audio&) = delete;
    Audio& operator=(const Audio&) = delete;
    Audio(Audio&&) = delete;
    Audio& operator=(Audio&&) = delete;

    void operator~()
    {
        CoUninitialize();
        if (pAudioClient != NULL)
            pAudioClient->Stop();

        CoTaskMemFree(pwfx);
        SAFE_RELEASE(pEnumerator)
            SAFE_RELEASE(pDevice)
            SAFE_RELEASE(pAudioClient)
            SAFE_RELEASE(pCaptureClient)
    }
};

