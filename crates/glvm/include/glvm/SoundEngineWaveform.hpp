// This file is part of Game Loop Versatile Modules (GLVM)
// Copyright © 2024 Maksim Manokhin a.k.a. Yuriorkis_Scream. Contacts:
// <fellfrostqtw@gmail.com> Author: Maksim Manokhin a.k.a. Yuriorkis_Scream
// License: http://opensource.org/licenses/MIT

#ifndef SOUND_ENGINE_WAVEFORM
#define SOUND_ENGINE_WAVEFORM

#include "glvm/ISoundEngine.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
// clang-format off
#include <windows.h>
#include <mmsystem.h>
// clang-format on

namespace GLVM::core::Sound {
class CSoundEngineWaveform: public ISoundEngine {
    HANDLE hData = NULL;
    HPSTR lpData = NULL;

    vector<CSoundSample*> tSound_Container;

public:
    void OpenDevice(const char* device) override;
    void CloseDevice() override;
    void SoundStream() override;
    void PlaybackSoundSample(CSoundSample& _sound_sample) override;
    void SetMasterVolume(long _lVolume) override;
    void CreateSoundSample(
        const char* filePath,
        u32 duration,
        u32 rate,
        float volume
    ) override;
    vector<CSoundSample*>& GetSoundContainer() override;
};
} // namespace GLVM::core::Sound

#endif
