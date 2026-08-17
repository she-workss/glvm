#ifndef ANIMATION_COMPONENT_HPP
#define ANIMATION_COMPONENT_HPP

#include <cstdint>

namespace GLVM::ecs::components {
struct animation {
    uint32_t currentAnimationFrame = 0;
    float frameAccumulator = 0.0f;
};
} // namespace GLVM::ecs::components

#endif
