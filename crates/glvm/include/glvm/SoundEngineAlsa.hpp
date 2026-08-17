// This file is part of Game Loop Versatile Modules (GLVM)
// Copyright © 2024 Maksim Manokhin a.k.a. Yuriorkis_Scream. Contacts:
// <fellfrostqtw@gmail.com> Author: Maksim Manokhin a.k.a. Yuriorkis_Scream
// License: http://opensource.org/licenses/MIT

#ifndef SOUND_ENGINE
#define SOUND_ENGINE

#include "glvm/Event.hpp"
#include "glvm/ISoundEngine.hpp"
#include "glvm/Vector.hpp"
#include "glvm/typenames.hpp"

#include <algorithm>
#include <alsa/asoundlib.h>
#include <alsa/pcm.h>
#include <bits/types/FILE.h>

namespace GLVM::core::Sound {
class CSoundEngineAlsa: public ISoundEngine {
    snd_pcm_t* pPcm;
    vector<CSoundSample*> tSound_Contaier;

public:
    void OpenDevice(const char* device) override;
    void CloseDevice() override;
    void SoundStream() override;
    void PlaybackSoundSample(CSoundSample& _sound_sample) override;
    void SetMasterVolume(long _lVolume) override;
    vector<CSoundSample*>& GetSoundContainer() override;
    void CreateSoundSample(
        const char* filePath,
        u32 duration,
        u32 rate,
        float volume
    ) override;

    ~CSoundEngineAlsa();
};
} // namespace GLVM::core::Sound

#endif
