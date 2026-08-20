#pragma once

#include "glvm/i_sound_engine.hpp"

namespace glvm::core::Sound {
class CSoundEngineFactory {
public:
    ISoundEngine* CreateSoundEngine();
};

} // namespace glvm::core::Sound
