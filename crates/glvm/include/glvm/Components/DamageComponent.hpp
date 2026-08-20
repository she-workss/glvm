#pragma once

namespace glvm::ecs::components {
struct damage {
    float maximumDamage;
    float minimumDamage;
    float criticalHitRate;
    float criticalModifier;
};
} // namespace glvm::ecs::components
