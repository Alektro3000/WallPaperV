#pragma once
#include "numeric"
#include "Audio.h"
#include "fft.cpp"


const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
const IID IID_IAudioClient = __uuidof(IAudioClient);
const IID IID_IAudioCaptureClient = __uuidof(IAudioCaptureClient);

Audio::Audio()
{
    CoInitialize(nullptr);
    HRESULT hr = CoCreateInstance(
        CLSID_MMDeviceEnumerator, NULL,
        CLSCTX_ALL, IID_IMMDeviceEnumerator,
        (void**)&pEnumerator);
    InitAudioCapture();
}
void Audio::InitAudioCapture(){
    HRESULT hr;
        hr = pEnumerator->GetDefaultAudioEndpoint(
            eRender, eConsole, &pDevice);
    EXIT_ON_ERROR(hr)

        hr = pDevice->Activate(
            IID_IAudioClient, CLSCTX_ALL,
            NULL, (void**)&pAudioClient);
    EXIT_ON_ERROR(hr)
    {

        hr = pAudioClient->GetMixFormat(reinterpret_cast<WAVEFORMATEX**>(&pwfx));
        EXIT_ON_ERROR(hr)
    }

    hr = pAudioClient->Initialize(
        AUDCLNT_SHAREMODE_SHARED,
        AUDCLNT_STREAMFLAGS_LOOPBACK,
        hnsRequestedDuration,
        0,
        &pwfx->Format,
        NULL);
    EXIT_ON_ERROR(hr)

        // Get the size of the allocated buffer.
        hr = pAudioClient->GetBufferSize(&bufferFrameCount);
    EXIT_ON_ERROR(hr)

        hr = pAudioClient->GetService(
            IID_IAudioCaptureClient,
            (void**)&pCaptureClient);
    EXIT_ON_ERROR(hr)


    hr = pAudioClient->Start();  // Start recording.
    EXIT_ON_ERROR(hr)
    return;
Exit:
    throw std::exception("Failed Initializing of Audio Capture");
};

void Audio::ReleaseAudioCapture() {

    if (pAudioClient != NULL)
        pAudioClient->Stop();

    CoTaskMemFree(pwfx);
        SAFE_RELEASE(pDevice)
        SAFE_RELEASE(pAudioClient)
        SAFE_RELEASE(pCaptureClient)
}

void Audio::operator~()
{
    SAFE_RELEASE(pEnumerator)
    ReleaseAudioCapture();
    CoUninitialize();
}
bool Audio::CheckActualCapture() {
    HRESULT hr;
    LPWSTR a = nullptr, b= nullptr;

    IMMDevice* pDev;
    hr = pEnumerator->GetDefaultAudioEndpoint(
        eRender, eConsole, &pDev);
    EXIT_ON_ERROR(hr)
    hr = pDev->GetId(&a);
    SAFE_RELEASE(pDev)
    EXIT_ON_ERROR(hr)
    hr = pDevice->GetId(&b);
    EXIT_ON_ERROR(hr)
    if (wcscmp(a, b) != 0)
        EXIT_ON_ERROR(hr)
    return false;
Exit:

    ReleaseAudioCapture();
    InitAudioCapture();
    CoTaskMemFree(a);
    CoTaskMemFree(b);
    return true;
}

void Audio::RecordSound()
{
    HRESULT hr;
    UINT32 packetLength = 0;
    hr = pCaptureClient->GetNextPacketSize(&packetLength);
    if ((packetLength == 0) || (hr != 0)) {
        if (CheckActualCapture())
            hr = pCaptureClient->GetNextPacketSize(&packetLength);
    }
    while (packetLength != 0)
    {
        DWORD flags;
        BYTE* pData;
        // Get the available data in the shared buffer.
        hr = pCaptureClient->GetBuffer(
            &pData,
            &numFramesAvailable,
            &flags, NULL, NULL);
        EXIT_ON_ERROR(hr)
            if (flags & AUDCLNT_BUFFERFLAGS_SILENT)
            {
                pData = NULL;  // Tell CopyData to write silence.
            }
            else
            {
                // Copy the available capture data to the audio sink.
                StoredData.resize(numFramesAvailable + StoredData.size());

                for (UINT32 i = 0; i < numFramesAvailable; i++)
                    StoredData[StoredData.size() + i - numFramesAvailable] = (reinterpret_cast<float*>(pData))[i * pwfx->Format.nChannels];

            }

        EXIT_ON_ERROR(hr)

            hr = pCaptureClient->ReleaseBuffer(numFramesAvailable);
        EXIT_ON_ERROR(hr)

            hr = pCaptureClient->GetNextPacketSize(&packetLength);
        EXIT_ON_ERROR(hr)
    }

Exit:
    return;
}

void Audio::ClearRecord()
{
    StoredData.clear();
}

std::vector<double> Audio::GetSound(int discr)
{
    RecordSound();
    if (StoredData.size() != 0)
    {
        int k = (int)StoredData.size();
        k--;
        k |= k >> 1;
        k |= k >> 2;
        k |= k >> 4;
        k |= k >> 8;
        k |= k >> 16;
        k++;
        k /= 2;
        StoredData.resize(k);
        auto rawInfo = FTran.evaluate(StoredData);

        auto herzToBucket = [discr](auto herz)
                {return (pow(log10(herz),3) * 1.38 - 3.05) * discr / 100.; };
        auto kToHerz = [k, SamplePerSecond = (pwfx->Format.nSamplesPerSec)](auto i)
            {return ((double)i / k) * SamplePerSecond; };

        out.resize(discr, 0);
        outWeight.resize(discr, 0);
        std::fill(out.begin(), out.end(), 0);
        std::fill(outWeight.begin(), outWeight.end(), 0);
        int mx = discr;
        double mxherz = 20000;
        for (int i = 1; i < k; ++i)
        {   
            double herz = kToHerz(i);
            auto id = herzToBucket(herz);
            if (id >= discr)
            {
                mx = i - 1;
                mxherz = kToHerz(i-1);
                break;
            }
            if (id >= 0)
                outWeight[(int)id]++;
        }
        int l = std::reduce(outWeight.begin()+1,outWeight.end(), 0);
        for (int i = 1; i < mx; ++i)
        {
            int id = (int)herzToBucket(kToHerz(i));
            if (id >= 0 && id < discr)
                out[id] = max(out[id], abs(rawInfo[i]));
        }
        for (int i = 1; i < discr; ++i)
            if (outWeight[i] != 0)
                out[i] = min((out[i] / k) * 35, 0.4);
        for (int i = 1; i < discr-1; ++i)
            if (outWeight[i] == 0)
                out[i] = (out[i - 1] + out[i + 1] )/ 2;
        return out;
    }
    return {};
}