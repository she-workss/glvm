#pragma once

#include <cstdint>

namespace glvm::ecs::arch {
#define ENTITY_ID_BITS 32
#define GENERATION_BITS 32

constexpr uint64_t entityBitsMask = (1ull << ENTITY_ID_BITS) - 1;

struct ComponentsIndices {
    enum Types : uint32_t {
        TRANSFORM_COMPONENT,
        RIGID_BODY_COMPONENT,
        MESH_COMPONENT,
        FONT_COMPONENT,
        COLLIDER_COMPONENT,
        COLLIDER_FLAGS_COMPONENT,
        MATERIAL_COMPONENT,
        VIEW_COMPONENT,
        HEALTH_COMPONENT,
        ANIMATION_COMPONENT,
        STATE_COMPONENT,
        ENEMY_COMPONENT,
        DAMAGE_COMPONENT,
        ATTACK_COMPONENT,
        INVENTORY_COMPONENT,
        DIRECTIONAL_LIGHT_COMPONENT,
        SPOT_LIGHT_COMPONENT,
        POINT_LIGHT_COMPONENT,
        ITEM_COMPONENT,
        MOVE_COMPONENT,
        PROJECTILE_BUNDLE_COMPONENT,
        ROTATION_COMPONENT,

        LEVEL_CHUNK_TAG_COMPONENT,
        PLAYER_TAG_COMPONENT,
        CROSSHAIR_TAG_COMPONENT,
        STATIC_MESH_TAG_COMPONENT,
        PROJECTILE_TAG_COMPONENT,

        COMPONENTS_COUNT
    };
};

constexpr uint64_t playerComponentMask =
    (1ull << ComponentsIndices::TRANSFORM_COMPONENT)
    | (1ull << ComponentsIndices::VIEW_COMPONENT)
    | (1ull << ComponentsIndices::COLLIDER_COMPONENT)
    | (1ull << ComponentsIndices::COLLIDER_FLAGS_COMPONENT)
    | (1ull << ComponentsIndices::MESH_COMPONENT)
    | (1ull << ComponentsIndices::RIGID_BODY_COMPONENT)
    | (1ull << ComponentsIndices::HEALTH_COMPONENT)
    | (1ull << ComponentsIndices::MATERIAL_COMPONENT)
    | (1ull << ComponentsIndices::MOVE_COMPONENT)
    | (1ull << ComponentsIndices::ATTACK_COMPONENT)
    | (1ull << ComponentsIndices::ANIMATION_COMPONENT)
    | (1ull << ComponentsIndices::FONT_COMPONENT)
    | (1ull << ComponentsIndices::ROTATION_COMPONENT)
    | (1ull << ComponentsIndices::PLAYER_TAG_COMPONENT);

constexpr uint64_t enemyComponentMask =
    (1ull << ComponentsIndices::TRANSFORM_COMPONENT)
    | (1ull << ComponentsIndices::ENEMY_COMPONENT)
    | (1ull << ComponentsIndices::STATE_COMPONENT)
    | (1ull << ComponentsIndices::FONT_COMPONENT)
    | (1ull << ComponentsIndices::ANIMATION_COMPONENT)
    | (1ull << ComponentsIndices::MATERIAL_COMPONENT)
    | (1ull << ComponentsIndices::MESH_COMPONENT)
    | (1ull << ComponentsIndices::COLLIDER_COMPONENT)
    | (1ull << ComponentsIndices::COLLIDER_FLAGS_COMPONENT)
    | (1ull << ComponentsIndices::HEALTH_COMPONENT)
    | (1ull << ComponentsIndices::RIGID_BODY_COMPONENT)
    | (1ull << ComponentsIndices::ATTACK_COMPONENT)
    | (1ull << ComponentsIndices::ROTATION_COMPONENT)
    | (1ull << ComponentsIndices::MOVE_COMPONENT);

constexpr uint64_t staticMeshComponentMask =
    (1ull << ComponentsIndices::TRANSFORM_COMPONENT)
    | (1ull << ComponentsIndices::COLLIDER_COMPONENT)
    | (1ull << ComponentsIndices::COLLIDER_FLAGS_COMPONENT)
    | (1ull << ComponentsIndices::MESH_COMPONENT)
    | (1ull << ComponentsIndices::MATERIAL_COMPONENT)
    | (1ull << ComponentsIndices::FONT_COMPONENT)
    | (1ull << ComponentsIndices::ROTATION_COMPONENT)
    | (1ull << ComponentsIndices::STATIC_MESH_TAG_COMPONENT);

constexpr uint64_t crosshairComponentMask =
    (1ull << ComponentsIndices::TRANSFORM_COMPONENT)
    | (1ull << ComponentsIndices::MESH_COMPONENT)
    | (1ull << ComponentsIndices::MATERIAL_COMPONENT)
    | (1ull << ComponentsIndices::CROSSHAIR_TAG_COMPONENT);

constexpr uint64_t itemComponentMask =
    (1ull << ComponentsIndices::TRANSFORM_COMPONENT)
    | (1ull << ComponentsIndices::COLLIDER_COMPONENT)
    | (1ull << ComponentsIndices::COLLIDER_FLAGS_COMPONENT)
    | (1ull << ComponentsIndices::MESH_COMPONENT)
    | (1ull << ComponentsIndices::RIGID_BODY_COMPONENT)
    | (1ull << ComponentsIndices::MATERIAL_COMPONENT)
    | (1ull << ComponentsIndices::ROTATION_COMPONENT)
    | (1ull << ComponentsIndices::MOVE_COMPONENT)
    | (1ull << ComponentsIndices::ITEM_COMPONENT);

constexpr uint64_t inventoryComponentMask =
    (1ull << ComponentsIndices::TRANSFORM_COMPONENT)
    | (1ull << ComponentsIndices::MESH_COMPONENT)
    | (1ull << ComponentsIndices::INVENTORY_COMPONENT)
    | (1ull << ComponentsIndices::MATERIAL_COMPONENT);

constexpr uint64_t directionalLightComponentMask =
    (1ull << ComponentsIndices::TRANSFORM_COMPONENT)
    | (1ull << ComponentsIndices::MESH_COMPONENT)
    | (1ull << ComponentsIndices::MATERIAL_COMPONENT)
    | (1ull << ComponentsIndices::DIRECTIONAL_LIGHT_COMPONENT);

constexpr uint64_t spotLightComponentMask =
    (1ull << ComponentsIndices::TRANSFORM_COMPONENT)
    | (1ull << ComponentsIndices::MESH_COMPONENT)
    | (1ull << ComponentsIndices::MATERIAL_COMPONENT)
    | (1ull << ComponentsIndices::SPOT_LIGHT_COMPONENT);

constexpr uint64_t pointLightComponentMask =
    (1ull << ComponentsIndices::TRANSFORM_COMPONENT)
    | (1ull << ComponentsIndices::MESH_COMPONENT)
    | (1ull << ComponentsIndices::MATERIAL_COMPONENT)
    | (1ull << ComponentsIndices::POINT_LIGHT_COMPONENT);

constexpr uint64_t levelChunkComponentMask =
    (1ull << ecs::arch::ComponentsIndices::TRANSFORM_COMPONENT)
    | (1ull << ecs::arch::ComponentsIndices::MATERIAL_COMPONENT)
    | (1ull << ecs::arch::ComponentsIndices::MESH_COMPONENT)
    | (1ull << ecs::arch::ComponentsIndices::COLLIDER_COMPONENT)
    | (1ull << ecs::arch::ComponentsIndices::COLLIDER_FLAGS_COMPONENT)
    | (1ull << ecs::arch::ComponentsIndices::ROTATION_COMPONENT)
    | (1ull << ecs::arch::ComponentsIndices::LEVEL_CHUNK_TAG_COMPONENT);

constexpr uint64_t projectileComponentMask =
    (1ull << ComponentsIndices::TRANSFORM_COMPONENT)
    | (1ull << ComponentsIndices::MESH_COMPONENT)
    | (1ull << ComponentsIndices::COLLIDER_COMPONENT)
    | (1ull << ComponentsIndices::COLLIDER_FLAGS_COMPONENT)
    | (1ull << ComponentsIndices::ROTATION_COMPONENT)
    | (1ull << ComponentsIndices::PROJECTILE_BUNDLE_COMPONENT)
    | (1ull << ComponentsIndices::PROJECTILE_TAG_COMPONENT);
}; // namespace glvm::ecs::arch
