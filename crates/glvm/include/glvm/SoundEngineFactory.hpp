#pragma once

#include "glvm/ISoundEngine.hpp"

namespace glvm::core::Sound {
class CSoundEngineFactory {
public:
    ISoundEngine* CreateSoundEngine();
};

} // namespace glvm::core::Sound
