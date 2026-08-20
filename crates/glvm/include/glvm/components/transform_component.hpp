#pragma once

#include "glvm/vertex_math.hpp"

namespace glvm::ecs::components {
struct transform {
    Vector<float, 3> position {0.0f, 0.0f, 0.0f};
    Vector<float, 3> forward {0.0f, 0.0f, 0.0f};
    float scale = 1.0f;
    float gravityAccumulator = 0.0f;
};
} // namespace glvm::ecs::components
