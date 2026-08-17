#ifndef COLLIDER_ARCHETYPE
#define COLLIDER_ARCHETYPE

#include "glvm/Components/ColliderComponent.hpp"
#include "glvm/Components/ColliderFlagsComponent.hpp"
#include "glvm/Globals.hpp"

namespace GLVM::ecs::arch {
constexpr uint32_t COLLIDER_ARCH_CHUNK_SIZE = ARCHETYPE_CHUNK_SIZE
    / (sizeof(GLVM::ecs::components::collider)
       + sizeof(GLVM::ecs::components::colliderFlags));

struct ColliderArch {
    components::collider colliders[COLLIDER_ARCH_CHUNK_SIZE];
    components::colliderFlags colliderFlags[COLLIDER_ARCH_CHUNK_SIZE];
};
}; // namespace GLVM::ecs::arch

#endif
