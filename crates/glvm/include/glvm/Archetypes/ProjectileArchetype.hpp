#pragma once

#include "glvm/ArchetypeECS/ArchECS_Types.hpp"
#include "glvm/ArchetypeECS/ArchetypeInterface.hpp"
#include "glvm/Components/AttackComponent.hpp"
#include "glvm/Components/ColliderComponent.hpp"
#include "glvm/Components/ColliderFlagsComponent.hpp"
#include "glvm/Components/DamageComponent.hpp"
#include "glvm/Components/FontComponent.hpp"
#include "glvm/Components/HealthComponent.hpp"
#include "glvm/Components/MaterialComponent.hpp"
#include "glvm/Components/ProjectileBundle.hpp"
#include "glvm/Components/ProjectileComponent.hpp"
#include "glvm/Components/RotationComponent.hpp"
#include "glvm/Components/TransformComponent.hpp"
#include "glvm/Components/VertexComponent.hpp"
#include "glvm/Globals.hpp"
#include "glvm/TagComponents/ProjectileTagComponent.hpp"

namespace glvm::ecs::arch {
constexpr uint32_t PROJECTILE_ARCH_CHUNK_SIZE = ARCHETYPE_CHUNK_SIZE
    / (sizeof(components::transform) + sizeof(components::mesh)
       + sizeof(components::collider) + sizeof(components::colliderFlags)
       + sizeof(components::rotation) + sizeof(ProjectileBundle)
       + sizeof(components::health) + sizeof(components::attack)
       + sizeof(components::font)
       + sizeof(tagComponents::projectileTagComponent));

struct ProjectileArchetype: Archetype {
    components::transform transforms[PROJECTILE_ARCH_CHUNK_SIZE];
    components::mesh meshes[PROJECTILE_ARCH_CHUNK_SIZE];
    components::collider colliders[PROJECTILE_ARCH_CHUNK_SIZE];
    components::colliderFlags colliderFlags[PROJECTILE_ARCH_CHUNK_SIZE];
    components::rotation rotations[PROJECTILE_ARCH_CHUNK_SIZE];
    ProjectileBundle projectileBundles[PROJECTILE_ARCH_CHUNK_SIZE];
    components::health heath[PROJECTILE_ARCH_CHUNK_SIZE];
    components::attack attacks[PROJECTILE_ARCH_CHUNK_SIZE];
    components::font fonts[PROJECTILE_ARCH_CHUNK_SIZE];
    tagComponents::projectileTagComponent
        projectileTagComponents[PROJECTILE_ARCH_CHUNK_SIZE];

    ProjectileArchetype() {
        components[ComponentsIndices::TRANSFORM_COMPONENT] = transforms;
        components[ComponentsIndices::MESH_COMPONENT] = meshes;
        components[ComponentsIndices::COLLIDER_COMPONENT] = colliders;
        components[ComponentsIndices::COLLIDER_FLAGS_COMPONENT] = colliderFlags;
        components[ComponentsIndices::ROTATION_COMPONENT] = rotations;
        components[ComponentsIndices::PROJECTILE_BUNDLE_COMPONENT] =
            projectileBundles;
        components[ComponentsIndices::HEALTH_COMPONENT] = heath;
        components[ComponentsIndices::ATTACK_COMPONENT] = attacks;
        components[ComponentsIndices::FONT_COMPONENT] = fonts;
        components[ComponentsIndices::PROJECTILE_TAG_COMPONENT] =
            projectileTagComponents;

        mask = (1ull << ComponentsIndices::TRANSFORM_COMPONENT)
            | (1ull << ComponentsIndices::MESH_COMPONENT)
            | (1ull << ComponentsIndices::COLLIDER_COMPONENT)
            | (1ull << ComponentsIndices::COLLIDER_FLAGS_COMPONENT)
            | (1ull << ComponentsIndices::ROTATION_COMPONENT)
            | (1ull << ComponentsIndices::PROJECTILE_BUNDLE_COMPONENT)
            | (1ull << ComponentsIndices::HEALTH_COMPONENT)
            | (1ull << ComponentsIndices::ATTACK_COMPONENT)
            | (1ull << ComponentsIndices::FONT_COMPONENT)
            | (1ull << ComponentsIndices::PROJECTILE_TAG_COMPONENT);

        componentIds[0] = ComponentsIndices::TRANSFORM_COMPONENT;
        componentIds[1] = ComponentsIndices::MESH_COMPONENT;
        componentIds[2] = ComponentsIndices::COLLIDER_COMPONENT;
        componentIds[3] = ComponentsIndices::COLLIDER_FLAGS_COMPONENT;
        componentIds[4] = ComponentsIndices::ROTATION_COMPONENT;
        componentIds[5] = ComponentsIndices::PROJECTILE_BUNDLE_COMPONENT;
        componentIds[6] = ComponentsIndices::HEALTH_COMPONENT;
        componentIds[7] = ComponentsIndices::ATTACK_COMPONENT;
        componentIds[8] = ComponentsIndices::FONT_COMPONENT;
        componentIds[9] = ComponentsIndices::PROJECTILE_TAG_COMPONENT;
        componentCount = 10;
    }
};
}; // namespace glvm::ecs::arch
