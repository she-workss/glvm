// This file is part of Game Loop Versatile Modules (GLVM)
// Copyright © 2024 Maksim Manokhin a.k.a. Yuriorkis_Scream. Contacts:
// <fellfrostqtw@gmail.com> Author: Maksim Manokhin a.k.a. Yuriorkis_Scream
// License: http://opensource.org/licenses/MIT

#ifndef TRANSFORM_COMPONENT_HPP
#define TRANSFORM_COMPONENT_HPP

#include "glvm/VertexMath.hpp"

namespace GLVM::ecs::components {
struct transform {
    vec3 position {0.0f, 0.0f, 0.0f};
    vec3 forward {0.0f, 0.0f, 0.0f};

    // float yaw = 0.0f;
    // float pitch = 0.0f;

    float scale = 1.0f;
    float gravityAccumulator = 0.0f;
    // bool hud = false;
    // float gravityAccumulator = 0.0f;

    // Animation
    // unsigned int currentAnimationFrame = 0;
    // float frameAccumulator = 0.0f;

    // bool gltf = true;
};
} // namespace GLVM::ecs::components

#endif
