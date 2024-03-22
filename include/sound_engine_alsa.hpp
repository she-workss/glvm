// This file is part of Game Loop Versatile Modules (GLVM)
// Copyright © 2024 Maksim Manokhin a.k.a. Yuriorkis_Scream. Contacts:
// <fellfrostqtw@gmail.com> Author: Maksim Manokhin a.k.a. Yuriorkis_Scream
// License: http://opensource.org/licenses/MIT
#pragma once

#include "event.hpp"
#include "i_sound_engine.hpp"
#include "vector.hpp"

#include <alsa/asoundlib.h>
#include <alsa/pcm.h>

namespace GLVM::core::Sound {
class CSoundEngineAlsa : public ISoundEngine {
    vector<CSoundSample *> tSound_Contaier;

public:
    void SoundStream() override;
    void PlaybackSoundSample(CSoundSample &_sound_sample) override;
    void SetMasterVolume(long _lVolume) override;
    vector<CSoundSample *> &GetSoundContainer() override;
};
} // namespace GLVM::core::Sound
