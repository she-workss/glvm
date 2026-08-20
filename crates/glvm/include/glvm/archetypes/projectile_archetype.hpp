#pragma once

#include "glvm/archetype_ecs/arch_ecs_types.hpp"
#include "glvm/archetype_ecs/archetype_interface.hpp"
#include "glvm/components/attack_component.hpp"
#include "glvm/components/collider_component.hpp"
#include "glvm/components/collider_flags_component.hpp"
#include "glvm/components/damage_component.hpp"
#include "glvm/components/font_component.hpp"
#include "glvm/components/health_component.hpp"
#include "glvm/components/material_component.hpp"
#include "glvm/components/projectile_bundle.hpp"
#include "glvm/components/projectile_component.hpp"
#include "glvm/components/rotation_component.hpp"
#include "glvm/components/transform_component.hpp"
#include "glvm/components/vertex_component.hpp"
#include "glvm/globals.hpp"
#include "glvm/tag_components/projectile_tag_component.hpp"

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
