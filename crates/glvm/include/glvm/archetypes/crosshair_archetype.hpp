#pragma once

#include "glvm/archetype_ecs/archetype_interface.hpp"
#include "glvm/components/material_component.hpp"
#include "glvm/components/transform_component.hpp"
#include "glvm/components/vertex_component.hpp"
#include "glvm/globals.hpp"
#include "glvm/tag_components/crosshair_tag_component.hpp"

namespace glvm::ecs::arch {
constexpr uint32_t CROSSHAIR_ARCH_CHUNK_SIZE = ARCHETYPE_CHUNK_SIZE
    / (sizeof(components::transform) + sizeof(components::mesh)
       + sizeof(components::material)
       + sizeof(tagComponents::crossHairTagComponent));

struct CrosshairArchetype: Archetype {
    components::transform transforms[CROSSHAIR_ARCH_CHUNK_SIZE];
    components::mesh meshes[CROSSHAIR_ARCH_CHUNK_SIZE];
    components::material materials[CROSSHAIR_ARCH_CHUNK_SIZE];
    tagComponents::crossHairTagComponent
        crosshairTagComponents[CROSSHAIR_ARCH_CHUNK_SIZE];

    CrosshairArchetype() {
        components[ComponentsIndices::TRANSFORM_COMPONENT] = transforms;
        components[ComponentsIndices::MESH_COMPONENT] = meshes;
        components[ComponentsIndices::MATERIAL_COMPONENT] = materials;
        components[ComponentsIndices::CROSSHAIR_TAG_COMPONENT] =
            crosshairTagComponents;

        mask = (1ull << ComponentsIndices::TRANSFORM_COMPONENT)
            | (1ull << ComponentsIndices::MESH_COMPONENT)
            | (1ull << ComponentsIndices::MATERIAL_COMPONENT)
            | (1ull << ComponentsIndices::CROSSHAIR_TAG_COMPONENT);

        componentIds[0] = ComponentsIndices::TRANSFORM_COMPONENT;
        componentIds[1] = ComponentsIndices::MESH_COMPONENT;
        componentIds[2] = ComponentsIndices::MATERIAL_COMPONENT;
        componentIds[3] = ComponentsIndices::CROSSHAIR_TAG_COMPONENT;
        componentCount = 4;
    }
};
}; // namespace glvm::ecs::arch
