#pragma once

#include "glvm/ArchetypeECS/ArchECS_Types.hpp"
#include "glvm/Vector.hpp"
#include "glvm/typenames.hpp"

namespace glvm::ecs::arch {
struct Archetype {
    virtual ~Archetype() = default;

    static constexpr uint32_t CAPACITY = 1024;

    entity entities[CAPACITY];
    uint32_t entityCount = 0;
    uint32_t componentIds[ComponentsIndices::COMPONENTS_COUNT] = {};
    uint32_t componentCount = 0;
    void* components[ComponentsIndices::COMPONENTS_COUNT] = {};
    componentMask mask = 0;

    uint32_t addEntity(entity entity_);
    entity removeEntity(uint32_t index);
};

struct EntityLocation {
    Archetype* arch;
    uint32_t index;
    static const u8 maxGridCellNumber = 8;
    u8 gridCellCounter = 0;
    vec3 gridCellIndicies[maxGridCellNumber];
    u32 cellEntityIndices[maxGridCellNumber];
    // Is entity has been moved or removed.
    bool isDirty = false;
};
}; // namespace glvm::ecs::arch
