#pragma once

#include "glvm/archetype_ecs/archetype_interface.hpp"
#include "glvm/components/inventory_component.hpp"
#include "glvm/components/material_component.hpp"
#include "glvm/components/transform_component.hpp"
#include "glvm/components/vertex_component.hpp"
#include "glvm/globals.hpp"

namespace glvm::ecs::arch {
constexpr uint32_t INVENTORY_ARCH_CHUNK_SIZE = ARCHETYPE_CHUNK_SIZE
    / (sizeof(components::transform) + sizeof(components::mesh)
       + sizeof(components::inventory) + sizeof(components::material));

struct InventoryArchetype: Archetype {
    components::transform transforms[INVENTORY_ARCH_CHUNK_SIZE];
    components::mesh meshes[INVENTORY_ARCH_CHUNK_SIZE];
    components::inventory invetories[INVENTORY_ARCH_CHUNK_SIZE];
    components::material materials[INVENTORY_ARCH_CHUNK_SIZE];

    InventoryArchetype() {
        components[ComponentsIndices::TRANSFORM_COMPONENT] = transforms;
        components[ComponentsIndices::MESH_COMPONENT] = meshes;
        components[ComponentsIndices::INVENTORY_COMPONENT] = invetories;
        components[ComponentsIndices::MATERIAL_COMPONENT] = materials;

        mask = (1ull << ComponentsIndices::TRANSFORM_COMPONENT)
            | (1ull << ComponentsIndices::MESH_COMPONENT)
            | (1ull << ComponentsIndices::INVENTORY_COMPONENT)
            | (1ull << ComponentsIndices::MATERIAL_COMPONENT);

        componentIds[0] = ComponentsIndices::TRANSFORM_COMPONENT;
        componentIds[1] = ComponentsIndices::MESH_COMPONENT;
        componentIds[2] = ComponentsIndices::INVENTORY_COMPONENT;
        componentIds[3] = ComponentsIndices::MATERIAL_COMPONENT;
        componentCount = 4;
    }
};
}; // namespace glvm::ecs::arch
