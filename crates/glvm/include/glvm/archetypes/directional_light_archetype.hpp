#pragma once

#include "glvm/archetype_ecs/archetype_interface.hpp"
#include "glvm/components/directional_light_component.hpp"
#include "glvm/components/material_component.hpp"
#include "glvm/components/transform_component.hpp"
#include "glvm/components/vertex_component.hpp"
#include "glvm/globals.hpp"

namespace glvm::ecs::arch {
constexpr uint32_t DIRECTIONAL_LIGHT_ARCH_CHUNK_SIZE = ARCHETYPE_CHUNK_SIZE
    / (sizeof(components::transform) + sizeof(components::mesh)
       + sizeof(components::material) + sizeof(components::directionalLight));

struct DirectionalLightArchetype: Archetype {
    components::transform transforms[DIRECTIONAL_LIGHT_ARCH_CHUNK_SIZE];
    components::mesh meshes[DIRECTIONAL_LIGHT_ARCH_CHUNK_SIZE];
    components::material materials[DIRECTIONAL_LIGHT_ARCH_CHUNK_SIZE];
    components::directionalLight
        directionalLights[DIRECTIONAL_LIGHT_ARCH_CHUNK_SIZE];

    DirectionalLightArchetype() {
        components[ComponentsIndices::TRANSFORM_COMPONENT] = transforms;
        components[ComponentsIndices::MESH_COMPONENT] = meshes;
        components[ComponentsIndices::MATERIAL_COMPONENT] = materials;
        components[ComponentsIndices::DIRECTIONAL_LIGHT_COMPONENT] =
            directionalLights;

        mask = (1ull << ComponentsIndices::TRANSFORM_COMPONENT)
            | (1ull << ComponentsIndices::MESH_COMPONENT)
            | (1ull << ComponentsIndices::MATERIAL_COMPONENT)
            | (1ull << ComponentsIndices::DIRECTIONAL_LIGHT_COMPONENT);

        componentIds[0] = ComponentsIndices::TRANSFORM_COMPONENT;
        componentIds[1] = ComponentsIndices::MESH_COMPONENT;
        componentIds[2] = ComponentsIndices::MATERIAL_COMPONENT;
        componentIds[3] = ComponentsIndices::DIRECTIONAL_LIGHT_COMPONENT;
        componentCount = 4;
    }
};
}; // namespace glvm::ecs::arch
