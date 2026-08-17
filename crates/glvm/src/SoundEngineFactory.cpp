// This file is part of Game Loop Versatile Modules (GLVM)
// Copyright © 2024 Maksim Manokhin a.k.a. Yuriorkis_Scream. Contacts:
// <fellfrostqtw@gmail.com> Author: Maksim Manokhin a.k.a. Yuriorkis_Scream
// License: http://opensource.org/licenses/MIT

#include "glvm/SoundEngineFactory.hpp"

#ifdef __linux__
#include "glvm/SoundEngineAlsa.hpp"
#endif

#ifdef _WIN32
#include "glvm/SoundEngineWaveform.hpp"
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

void CSoundEngineWaveform::OpenDevice(const char* device) {}

void CSoundEngineWaveform::CloseDevice() {}

void CSoundEngineWaveform::CreateSoundSample(
    const char* filePath,
    u32 duration,
    u32 rate,
    float volume
) {
    CSoundSample* sample = new CSoundSample {filePath, duration, rate, volume};
    tSound_Container.Push(sample);
}
} // namespace GLVM::core::Sound
