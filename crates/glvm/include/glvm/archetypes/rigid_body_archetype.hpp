#pragma once

#include "glvm/components/rigid_body_component.hpp"
#include "glvm/components/transform_component.hpp"
#include "glvm/globals.hpp"

namespace glvm::ecs::arch {
constexpr uint32_t RIGID_BODY_ARCH_CHUNK_SIZE = ARCHETYPE_CHUNK_SIZE
    / (sizeof(glvm::ecs::components::transform)
       + sizeof(glvm::ecs::components::rigidBody));

struct RigidBodyArch {
    components::transform transforms[RIGID_BODY_ARCH_CHUNK_SIZE];
    components::rigidBody rigidBodies[RIGID_BODY_ARCH_CHUNK_SIZE];
};
}; // namespace glvm::ecs::arch
