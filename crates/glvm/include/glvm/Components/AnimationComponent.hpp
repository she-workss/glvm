#pragma once

#include <cstdint>

namespace glvm::ecs::components {
struct animation {
    uint32_t currentAnimationFrame = 0;
    float frameAccumulator = 0.0f;
};
} // namespace glvm::ecs::components
