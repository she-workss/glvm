// This file is part of Game Loop Versatile Modules (GLVM)
// Copyright © 2024 Maksim Manokhin a.k.a. Yuriorkis_Scream. Contacts:
// <fellfrostqtw@gmail.com> Author: Maksim Manokhin a.k.a. Yuriorkis_Scream
// License: http://opensource.org/licenses/MIT

#ifndef SOUND_ENGINE_WAVEFORM
#define SOUND_ENGINE_WAVEFORM

#include "i_sound_engine.hpp"
#include <windows.h>

#include <mmsystem.h>

namespace GLVM::core::Sound {
class CSoundEngineWaveform : public ISoundEngine {
    // Handle of waveform data memory
    HANDLE hData = NULL;

    // Pointer to waveform data memory
    HPSTR lpData = NULL;

    vector<CSoundSample *> tSound_Container;

public:
    void SoundStream() override;
    void PlaybackSoundSample(CSoundSample &_sound_sample) override;
    void SetMasterVolume(long _lVolume) override;
    vector<CSoundSample *> &GetSoundContainer() override;
};
} // namespace GLVM::core::Sound

#endif
