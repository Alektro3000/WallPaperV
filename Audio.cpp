#include "Audio.h"
#include <complex.h>
#include <math.h>
#include <cmath>
#include <iomanip>
#include <algorithm>
#include <bit>
#include <numeric>
#pragma once


typedef std::complex<double> ftype;


template<typename T>
std::vector<ftype> fft(std::vector<T> p, ftype w) {
    int n = p.size();
    if (n == 1)
        return { p[0] };

    std::vector<T> AB[2];
    for (int i = 0; i < n; i++)
        AB[i % 2].push_back(p[i]);
    auto A = fft(AB[0], w * w);
    auto B = fft(AB[1], w * w);
    std::vector<ftype> res(n);
    ftype wt = 1;
    int k = n / 2;
    for (int i = 0; i < n; i++) {
        res[i] = A[i % k] + wt * B[i % k];
        wt *= w;
    }
    return res;
    
}
//N must be Nth pow of 2    
template<typename T>
std::vector<ftype> evaluate(std::vector<T> p) {
    return fft(p, std::polar(1., 2 * (3.1415926) / p.size()));
}

const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
const IID IID_IAudioClient = __uuidof(IAudioClient);
const IID IID_IAudioCaptureClient = __uuidof(IAudioCaptureClient);

Audio::Audio()
{
    HRESULT hr;
    CoInitialize(nullptr);
    hr = CoCreateInstance(
        CLSID_MMDeviceEnumerator, NULL,
        CLSCTX_ALL, IID_IMMDeviceEnumerator,
        (void**)&pEnumerator);
    EXIT_ON_ERROR(hr)

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


    // Calculate the actual duration of the allocated buffer.
    hnsActualDuration = (double)REFTIMES_PER_SEC *
    bufferFrameCount / pwfx->Format.nSamplesPerSec;

    hr = pAudioClient->Start();  // Start recording.
    EXIT_ON_ERROR(hr)
    return;
Exit:
    throw std::exception("Failed Construction of Audio Capture");
};


std::vector<double> Audio::GetSound(int discr)
{
    HRESULT hr;
    UINT32 packetLength = 0;
    BYTE* pData;
    DWORD flags;
    int totalframes = 0;
    long long k = 0;
    static_assert(sizeof(k) == 8);
    hr = pCaptureClient->GetNextPacketSize(&packetLength);
    std::vector<float> data;
    EXIT_ON_ERROR(hr)
    while (packetLength != 0)
    {
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
                data.resize(numFramesAvailable + data.size());

                for (int i = 0; i < numFramesAvailable; i++)
                    data[data.size()+i-numFramesAvailable] = (reinterpret_cast<float*>(pData))[i * 2];
               
            }

        EXIT_ON_ERROR(hr)

            hr = pCaptureClient->ReleaseBuffer(numFramesAvailable);
        EXIT_ON_ERROR(hr)

            hr = pCaptureClient->GetNextPacketSize(&packetLength);
        EXIT_ON_ERROR(hr)
    }
    if (data.size() != 0)
    {
        int k = data.size();
        k--;
        k |= k >> 1;
        k |= k >> 2;
        k |= k >> 4;
        k |= k >> 8;
        k |= k >> 16;
        k |= k >> 32;
        k++;
        k /= 2;
        data.resize(k);
        auto rawInfo = evaluate(data);
        static_assert(sizeof(float) == 4);
        //auto herzToBucket = [](auto herz)
        //    {return log(pow(herz / 25,2) / (700)) / log(1.067); };
        //auto herzToBucket = [](auto herz)
        //    {return (herz-20)/100; };

        //auto herzToBucket = [](auto herz)
        //    {return log(herz / 25) / log(1.067); };
        auto herzToBucket = [](auto herz)
            {return log(pow(herz / 25,1.3) / (7.6)) / log(1.067); };

        auto kToHerz = [k, SamplePerSecond = (pwfx->Format.nSamplesPerSec)](auto i)
            {return ((double)i / k) * SamplePerSecond; };

        auto idk = [kToHerz, herzToBucket](auto i) {return herzToBucket(kToHerz(i)); };

        out.resize(discr, 0);
        outWeight.resize(discr, 0);
        std::fill(out.begin(), out.end(), 0);
        std::fill(outWeight.begin(), outWeight.end(), 0);
        int mx = 0;
        int mxherz = 0;
        for (int i = 1; i < k; ++i)
        {   
            double herz = kToHerz(i);
            int id = herzToBucket(herz);
            if (id >= discr)
            {
                mx = i - 1;
                mxherz = kToHerz(i-1);
                break;
            }
            if (id >= 0)
                outWeight[id]++;
        }
        int l = std::reduce(outWeight.begin()+1,outWeight.end(), 0);
        for (int i = 1; i < mx; ++i)
        {
            int id = idk(i);
            if (id >= 0 && id < discr)
                out[id] = max(out[id], abs(rawInfo[i]));
        }
        for (int i = 1; i < discr; ++i)
            if (outWeight[i] != 0)
                out[i] = min((out[i] / k) * 35,0.4);
        return out;
    }
Exit:
    return {};
}