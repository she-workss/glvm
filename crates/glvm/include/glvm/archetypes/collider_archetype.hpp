#pragma once

#include "glvm/components/collider_component.hpp"
#include "glvm/components/collider_flags_component.hpp"
#include "glvm/globals.hpp"

namespace glvm::ecs::arch {
constexpr uint32_t COLLIDER_ARCH_CHUNK_SIZE = ARCHETYPE_CHUNK_SIZE
    / (sizeof(glvm::ecs::components::collider)
       + sizeof(glvm::ecs::components::colliderFlags));

struct ColliderArch {
    components::collider colliders[COLLIDER_ARCH_CHUNK_SIZE];
    components::colliderFlags colliderFlags[COLLIDER_ARCH_CHUNK_SIZE];
};
}; // namespace glvm::ecs::arch
