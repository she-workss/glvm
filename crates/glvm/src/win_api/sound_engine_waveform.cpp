#include "glvm/sound_engine_waveform.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
// clang-format off
#include <windows.h>
#include <mmeapi.h>
// clang-format on

#include <cstdio>
#include <fstream>
#include <iostream>

namespace glvm::core::Sound {
void CSoundEngineWaveform::SoundStream() {
    for (unsigned int i = 0; i < tSound_Container.size(); ++i) {
        PlaybackSoundSample(*tSound_Container[i]);
        tSound_Container.erase(tSound_Container.begin() + i);
    }
}

void CSoundEngineWaveform::PlaybackSoundSample(CSoundSample& _sound_sample) {
    HWAVEOUT hWaveOut;
    WAVEHDR lpWaveHdr {};
    WAVEFORMATEX Format;
    Format.wFormatTag = WAVE_FORMAT_PCM;
    Format.nChannels = 2;
    Format.nSamplesPerSec = _sound_sample.uiRate_;
    Format.nAvgBytesPerSec = Format.nSamplesPerSec * Format.nChannels * 2;
    // Change this field first if got any problems.
    Format.nBlockAlign = 4;
    Format.wBitsPerSample = 16;
    Format.cbSize = 0;
    // Open a waveform device for output using window callback.
    unsigned int rc = 0;
    rc = waveOutOpen(&hWaveOut, WAVE_MAPPER, &Format, 0L, 0L, 0L);
    if (rc != MMSYSERR_NOERROR) {
        std::cerr << "waveOutOpen: " << "error code: " << rc << std::endl;
        std::exit(-1);
    }

    std::ifstream file(
        _sound_sample.kPath_to_File_,
        std::ios_base::binary | std::ios_base::in
    );
    if (!file) {
        std::cerr << "Fail to open file." << std::endl;
        std::exit(-1);
    }

    char* buf = (char*)malloc(Format.nAvgBytesPerSec * 2);
    while (true) {
        file.read(buf, Format.nAvgBytesPerSec * 2);
        if (file.gcount() == 0) {
            break;
        }

        lpWaveHdr.lpData = buf;
        lpWaveHdr.dwBufferLength = file.gcount();
        lpWaveHdr.dwFlags = 0L;
        lpWaveHdr.dwLoops = 0L;
        waveOutPrepareHeader(hWaveOut, &lpWaveHdr, sizeof(WAVEHDR));
        waveOutWrite(hWaveOut, &lpWaveHdr, sizeof(WAVEHDR));
        Sleep((lpWaveHdr.dwBufferLength * 1000) / (Format.nAvgBytesPerSec * 2));
        waveOutUnprepareHeader(hWaveOut, &lpWaveHdr, sizeof(WAVEHDR));
    }

    free(buf);
    waveOutClose(hWaveOut);
}

void CSoundEngineWaveform::SetMasterVolume(long _lVolume) {}

std::vector<CSoundSample*>& CSoundEngineWaveform::GetSoundContainer() {
    return tSound_Container;
}
} // namespace glvm::core::Sound
