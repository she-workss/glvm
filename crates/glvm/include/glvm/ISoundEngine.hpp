#pragma once

#include "glvm/Vector.hpp"
#include "glvm/typenames.hpp"

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
    virtual vector<CSoundSample*>& GetSoundContainer() = 0;
    virtual void PlaybackSoundSample(CSoundSample& _sound_sample) = 0;
    virtual void SetMasterVolume(long _lVolume) = 0;
    virtual void SoundStream() = 0;
    virtual void CreateSoundSample(
        const char* filePath,
        u32 duration,
        u32 rate,
        float volume
    ) = 0;
};
} // namespace glvm::core::Sound
