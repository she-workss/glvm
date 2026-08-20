#pragma once

#include "glvm/Event.hpp"
#include "glvm/VertexMath.hpp"

namespace glvm::ecs::components {
struct move {
    core::EEvents eEvent_ = core::EEvents::eDEFAULT;
    vec3 frameMovement {0.0f, 0.0f, 0.0f};
    vec3 gravity {0.0f, 0.0f, 0.0f};
};
} // namespace glvm::ecs::components
