#pragma once

#include "glvm/vertex_math.hpp"

namespace glvm::ecs::components {
class beholder {
public:
    Vector<float, 3> Position {0.0f, 0.0f, 0.0f};
    Vector<float, 3> forward {0.0f, 0.0, 0.0f};
};
} // namespace glvm::ecs::components
