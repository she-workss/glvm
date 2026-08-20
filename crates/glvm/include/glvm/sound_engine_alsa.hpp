#pragma once

#include "glvm/event.hpp"
#include "glvm/i_sound_engine.hpp"
#include "glvm/typenames.hpp"

#include <algorithm>
#include <alsa/asoundlib.h>
#include <alsa/pcm.h>
#include <bits/types/FILE.h>
#include <vector>

namespace glvm::core::Sound {
class CSoundEngineAlsa: public ISoundEngine {
    snd_pcm_t* pPcm;
    std::vector<CSoundSample*> tSound_Contaier;

public:
    void OpenDevice(const char* device) override;
    void CloseDevice() override;
    void SoundStream() override;
    void PlaybackSoundSample(CSoundSample& _sound_sample) override;
    void SetMasterVolume(long _lVolume) override;
    std::vector<CSoundSample*>& GetSoundContainer() override;
    void CreateSoundSample(
        const char* filePath,
        uint32_t duration,
        uint32_t rate,
        float volume
    ) override;

    ~CSoundEngineAlsa();
};
} // namespace glvm::core::Sound
