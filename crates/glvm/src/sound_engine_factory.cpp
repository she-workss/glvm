#include "glvm/sound_engine_factory.hpp"

#ifdef __linux__
#include "glvm/sound_engine_alsa.hpp"
#endif

#ifdef _WIN32
#include "glvm/sound_engine_waveform.hpp"
#endif

namespace glvm::core::Sound {
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
    uint32_t duration,
    uint32_t rate,
    float volume
) {
    CSoundSample* sample = new CSoundSample {filePath, duration, rate, volume};
    tSound_Container.push_back(sample);
}
} // namespace glvm::core::Sound
