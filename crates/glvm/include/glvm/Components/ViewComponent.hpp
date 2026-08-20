#pragma once

#include "glvm/VertexMath.hpp"

namespace glvm::ecs::components {
class beholder {
public:
    vec3 Position {0.0f, 0.0f, 0.0f};
    vec3 forward {0.0f, 0.0, 0.0f};
};
} // namespace glvm::ecs::components
