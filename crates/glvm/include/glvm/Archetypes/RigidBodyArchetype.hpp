#ifndef RIGID_BODY_ARCHETYPE
#define RIGID_BODY_ARCHETYPE

#include "glvm/Components/RigidBodyComponent.hpp"
#include "glvm/Components/TransformComponent.hpp"
#include "glvm/Globals.hpp"

namespace GLVM::ecs::arch {
constexpr uint32_t RIGID_BODY_ARCH_CHUNK_SIZE = ARCHETYPE_CHUNK_SIZE
    / (sizeof(GLVM::ecs::components::transform)
       + sizeof(GLVM::ecs::components::rigidBody));

struct RigidBodyArch {
    components::transform transforms[RIGID_BODY_ARCH_CHUNK_SIZE];
    components::rigidBody rigidBodies[RIGID_BODY_ARCH_CHUNK_SIZE];
};
}; // namespace GLVM::ecs::arch

#endif
