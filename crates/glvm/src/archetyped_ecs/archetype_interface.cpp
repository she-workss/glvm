#include "glvm/archetype_ecs/archetype_interface.hpp"

#include "glvm/components/animation_component.hpp"
#include "glvm/components/attack_component.hpp"
#include "glvm/components/collider_component.hpp"
#include "glvm/components/collider_flags_component.hpp"
#include "glvm/components/damage_component.hpp"
#include "glvm/components/directional_light_component.hpp"
#include "glvm/components/enemy_component.hpp"
#include "glvm/components/font_component.hpp"
#include "glvm/components/health_component.hpp"
#include "glvm/components/inventory_component.hpp"
#include "glvm/components/item_component.hpp"
#include "glvm/components/material_component.hpp"
#include "glvm/components/move_component.hpp"
#include "glvm/components/point_light_component.hpp"
#include "glvm/components/projectile_bundle.hpp"
#include "glvm/components/projectile_component.hpp"
#include "glvm/components/rigid_body_component.hpp"
#include "glvm/components/spot_light_component.hpp"
#include "glvm/components/state_component.hpp"
#include "glvm/components/transform_component.hpp"
#include "glvm/components/vertex_component.hpp"
#include "glvm/components/view_component.hpp"
#include "glvm/tag_components/level_chunk_tag_component.hpp"
#include "glvm/tag_components/player_tag_component.hpp"
#include "glvm/tag_components/projectile_tag_component.hpp"

#include <cassert>

namespace glvm::ecs::arch {
uint32_t Archetype::addEntity(uint64_t entity_) {
    uint32_t index = entityCount++;
    assert(index < CAPACITY);
    entities[index] = entity_;

    return index;
}

// Swap-remove.
uint64_t Archetype::removeEntity(uint32_t index) {
    uint32_t last = entityCount - 1;

    for (uint32_t i = 0; i < componentCount; ++i) {
        const uint32_t componentId = componentIds[i];

        switch (componentId) {
            case ComponentsIndices::TRANSFORM_COMPONENT:
                static_cast<components::transform*>(
                    components[componentId]
                )[index] =
                    static_cast<components::transform*>(
                        components[componentId]
                    )[last];
                break;
            case ComponentsIndices::RIGID_BODY_COMPONENT:
                static_cast<components::rigidBody*>(
                    components[componentId]
                )[index] =
                    static_cast<components::rigidBody*>(
                        components[componentId]
                    )[last];
                break;
            case ComponentsIndices::MESH_COMPONENT:
                static_cast<components::mesh*>(components[componentId])[index] =
                    static_cast<components::mesh*>(
                        components[componentId]
                    )[last];
                break;
            case ComponentsIndices::FONT_COMPONENT:
                static_cast<components::font*>(components[componentId])[index] =
                    static_cast<components::font*>(
                        components[componentId]
                    )[last];
                break;
            case ComponentsIndices::COLLIDER_COMPONENT:
                static_cast<components::collider*>(
                    components[componentId]
                )[index] =
                    static_cast<components::collider*>(
                        components[componentId]
                    )[last];
                break;
            case ComponentsIndices::COLLIDER_FLAGS_COMPONENT:
                static_cast<components::colliderFlags*>(
                    components[componentId]
                )[index] =
                    static_cast<components::colliderFlags*>(
                        components[componentId]
                    )[last];
                break;
            case ComponentsIndices::MATERIAL_COMPONENT:
                static_cast<components::material*>(
                    components[componentId]
                )[index] =
                    static_cast<components::material*>(
                        components[componentId]
                    )[last];
                break;
            case ComponentsIndices::VIEW_COMPONENT:
                static_cast<components::beholder*>(
                    components[componentId]
                )[index] =
                    static_cast<components::beholder*>(
                        components[componentId]
                    )[last];
                break;
            case ComponentsIndices::HEALTH_COMPONENT:
                static_cast<components::health*>(
                    components[componentId]
                )[index] =
                    static_cast<components::health*>(
                        components[componentId]
                    )[last];
                break;
            case ComponentsIndices::ANIMATION_COMPONENT:
                static_cast<components::animation*>(
                    components[componentId]
                )[index] =
                    static_cast<components::animation*>(
                        components[componentId]
                    )[last];
                break;
            case ComponentsIndices::STATE_COMPONENT:
                static_cast<components::state*>(components[componentId])[index] =
                    static_cast<components::state*>(
                        components[componentId]
                    )[last];
                break;
            case ComponentsIndices::ENEMY_COMPONENT:
                static_cast<components::enemy*>(components[componentId])[index] =
                    static_cast<components::enemy*>(
                        components[componentId]
                    )[last];
                break;
            case ComponentsIndices::DAMAGE_COMPONENT:
                static_cast<components::damage*>(
                    components[componentId]
                )[index] =
                    static_cast<components::damage*>(
                        components[componentId]
                    )[last];
                break;
            case ComponentsIndices::ATTACK_COMPONENT:
                static_cast<components::attack*>(
                    components[componentId]
                )[index] =
                    static_cast<components::attack*>(
                        components[componentId]
                    )[last];
                break;
            case ComponentsIndices::INVENTORY_COMPONENT:
                static_cast<components::inventory*>(
                    components[componentId]
                )[index] =
                    static_cast<components::inventory*>(
                        components[componentId]
                    )[last];
                break;
            case ComponentsIndices::DIRECTIONAL_LIGHT_COMPONENT:
                static_cast<components::directionalLight*>(
                    components[componentId]
                )[index] =
                    static_cast<components::directionalLight*>(
                        components[componentId]
                    )[last];
                break;
            case ComponentsIndices::SPOT_LIGHT_COMPONENT:
                static_cast<components::spotLight*>(
                    components[componentId]
                )[index] =
                    static_cast<components::spotLight*>(
                        components[componentId]
                    )[last];
                break;
            case ComponentsIndices::POINT_LIGHT_COMPONENT:
                static_cast<components::pointLight*>(
                    components[componentId]
                )[index] =
                    static_cast<components::pointLight*>(
                        components[componentId]
                    )[last];
                break;
            case ComponentsIndices::ITEM_COMPONENT:
                static_cast<components::item*>(components[componentId])[index] =
                    static_cast<components::item*>(
                        components[componentId]
                    )[last];
                break;
            case ComponentsIndices::MOVE_COMPONENT:
                static_cast<components::move*>(components[componentId])[index] =
                    static_cast<components::move*>(
                        components[componentId]
                    )[last];
                break;
            case ComponentsIndices::PROJECTILE_BUNDLE_COMPONENT:
                static_cast<ProjectileBundle*>(components[componentId])[index] =
                    static_cast<ProjectileBundle*>(
                        components[componentId]
                    )[last];
                break;
            case ComponentsIndices::LEVEL_CHUNK_TAG_COMPONENT:
                static_cast<tagComponents::levelChunkTagComponent*>(
                    components[componentId]
                )[index] =
                    static_cast<tagComponents::levelChunkTagComponent*>(
                        components[componentId]
                    )[last];
                break;
            case ComponentsIndices::PROJECTILE_TAG_COMPONENT:
                static_cast<tagComponents::projectileTagComponent*>(
                    components[componentId]
                )[index] =
                    static_cast<tagComponents::projectileTagComponent*>(
                        components[componentId]
                    )[last];
                break;
            case ComponentsIndices::PLAYER_TAG_COMPONENT:
                static_cast<tagComponents::playerTagComponent*>(
                    components[componentId]
                )[index] =
                    static_cast<tagComponents::playerTagComponent*>(
                        components[componentId]
                    )[last];
                break;
        }
    }

    uint64_t moved = entities[last];
    entities[index] = moved;
    --entityCount;

    return moved;
}
}; // namespace glvm::ecs::arch
