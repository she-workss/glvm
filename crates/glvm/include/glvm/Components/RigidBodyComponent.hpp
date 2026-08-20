#pragma once

#include "glvm/VertexMath.hpp"

namespace glvm::ecs::components {
class rigidBody {
public:
    float fMass_ = 0.0f;
    float jumpAccumulator = 0.0f;
};
} // namespace glvm::ecs::components
