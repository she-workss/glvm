#pragma once

#include "glvm/archetype_ecs/arch_ecs_types.hpp"
#include "glvm/archetype_ecs/archetype_interface.hpp"
#include "glvm/components/collider_component.hpp"
#include "glvm/components/collider_flags_component.hpp"
#include "glvm/components/item_component.hpp"
#include "glvm/components/material_component.hpp"
#include "glvm/components/move_component.hpp"
#include "glvm/components/rigid_body_component.hpp"
#include "glvm/components/rotation_component.hpp"
#include "glvm/components/transform_component.hpp"
#include "glvm/components/vertex_component.hpp"
#include "glvm/globals.hpp"

namespace glvm::ecs::arch {
constexpr uint32_t ITEM_ARCH_CHUNK_SIZE = ARCHETYPE_CHUNK_SIZE
    / (sizeof(components::transform) + sizeof(components::collider)
       + sizeof(components::colliderFlags) + sizeof(components::mesh)
       + sizeof(components::rigidBody) + sizeof(components::material)
       + sizeof(components::rotation) + sizeof(components::move)
       + sizeof(components::item));

struct ItemArchetype: Archetype {
    components::transform transforms[ITEM_ARCH_CHUNK_SIZE];
    components::collider colliders[ITEM_ARCH_CHUNK_SIZE];
    components::colliderFlags colliderFlags[ITEM_ARCH_CHUNK_SIZE];
    components::mesh meshes[ITEM_ARCH_CHUNK_SIZE];
    components::rigidBody rigidBodies[ITEM_ARCH_CHUNK_SIZE];
    components::material materials[ITEM_ARCH_CHUNK_SIZE];
    components::rotation rotations[ITEM_ARCH_CHUNK_SIZE];
    components::move moves[ITEM_ARCH_CHUNK_SIZE];
    components::item items[ITEM_ARCH_CHUNK_SIZE];

    ItemArchetype() {
        components[ComponentsIndices::TRANSFORM_COMPONENT] = transforms;
        components[ComponentsIndices::COLLIDER_COMPONENT] = colliders;
        components[ComponentsIndices::COLLIDER_FLAGS_COMPONENT] = colliderFlags;
        components[ComponentsIndices::MESH_COMPONENT] = meshes;
        components[ComponentsIndices::RIGID_BODY_COMPONENT] = rigidBodies;
        components[ComponentsIndices::MATERIAL_COMPONENT] = materials;
        components[ComponentsIndices::ROTATION_COMPONENT] = rotations;
        components[ComponentsIndices::MOVE_COMPONENT] = moves;
        components[ComponentsIndices::ITEM_COMPONENT] = items;

        mask = (1ull << ComponentsIndices::TRANSFORM_COMPONENT)
            | (1ull << ComponentsIndices::COLLIDER_COMPONENT)
            | (1ull << ComponentsIndices::COLLIDER_FLAGS_COMPONENT)
            | (1ull << ComponentsIndices::MESH_COMPONENT)
            | (1ull << ComponentsIndices::RIGID_BODY_COMPONENT)
            | (1ull << ComponentsIndices::MATERIAL_COMPONENT)
            | (1ull << ComponentsIndices::ROTATION_COMPONENT)
            | (1ull << ComponentsIndices::MOVE_COMPONENT)
            | (1ull << ComponentsIndices::ITEM_COMPONENT);

        componentIds[0] = ComponentsIndices::TRANSFORM_COMPONENT;
        componentIds[1] = ComponentsIndices::COLLIDER_COMPONENT;
        componentIds[2] = ComponentsIndices::COLLIDER_FLAGS_COMPONENT;
        componentIds[3] = ComponentsIndices::MESH_COMPONENT;
        componentIds[4] = ComponentsIndices::RIGID_BODY_COMPONENT;
        componentIds[5] = ComponentsIndices::MATERIAL_COMPONENT;
        componentIds[6] = ComponentsIndices::ROTATION_COMPONENT;
        componentIds[7] = ComponentsIndices::MOVE_COMPONENT;
        componentIds[8] = ComponentsIndices::ITEM_COMPONENT;
        componentCount = 9;
    }
};
}; // namespace glvm::ecs::arch
