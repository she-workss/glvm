#pragma once

#include "glvm/event.hpp"
#include "glvm/vertex_math.hpp"

namespace glvm::ecs::components {
struct move {
    core::EEvents eEvent_ = core::EEvents::eDEFAULT;
    Vector<float, 3> frameMovement {0.0f, 0.0f, 0.0f};
    Vector<float, 3> gravity {0.0f, 0.0f, 0.0f};
};
} // namespace glvm::ecs::components
