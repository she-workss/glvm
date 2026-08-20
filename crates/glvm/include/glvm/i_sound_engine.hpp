#pragma once

#include "glvm/typenames.hpp"

#include <vector>

namespace glvm::core::Sound {
struct CSoundSample {
    const char* kPath_to_File_;
    unsigned int uiDuration_;
    unsigned int uiRate_;
    float volume;
};

class ISoundEngine {
public:
    virtual ~ISoundEngine() {}

    virtual void OpenDevice(const char* device) = 0;
    virtual void CloseDevice() = 0;
    virtual std::vector<CSoundSample*>& GetSoundContainer() = 0;
    virtual void PlaybackSoundSample(CSoundSample& _sound_sample) = 0;
    virtual void SetMasterVolume(long _lVolume) = 0;
    virtual void SoundStream() = 0;
    virtual void CreateSoundSample(
        const char* filePath,
        uint32_t duration,
        uint32_t rate,
        float volume
    ) = 0;
};
} // namespace glvm::core::Sound
