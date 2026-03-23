// This file is part of Game Loop Versatile Modules (GLVM)
// Copyright © 2024 Maksim Manokhin a.k.a. Yuriorkis_Scream. Contacts:
// <fellfrostqtw@gmail.com> Author: Maksim Manokhin a.k.a. Yuriorkis_Scream
// License: http://opensource.org/licenses/MIT

#include "glvm/sound_engine_factory.hpp"

#ifdef __linux__
#include "glvm/sound_engine_alsa.hpp"
#endif

#ifdef _WIN32
#include "glvm/sound_engine_waveform.hpp"
#endif

namespace GLVM::core::Sound {
ISoundEngine* CSoundEngineFactory::CreateSoundEngine() {
#ifdef __linux__
    return new CSoundEngineAlsa;
#endif

#ifdef _WIN32
    return new CSoundEngineWaveform;
#endif
}
} // namespace GLVM::core::Sound
