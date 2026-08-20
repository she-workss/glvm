#pragma once

#include "glvm/VertexMath.hpp"

namespace glvm::ecs::components {
struct transform {
    vec3 position {0.0f, 0.0f, 0.0f};
    vec3 forward {0.0f, 0.0f, 0.0f};
    float scale = 1.0f;
    float gravityAccumulator = 0.0f;
};
} // namespace glvm::ecs::components
