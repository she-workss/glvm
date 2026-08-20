#pragma once

#include "glvm/archetype_ecs/arch_ecs_types.hpp"
#include "glvm/vertex_math.hpp"
#include "glvm/typenames.hpp"

#include <vector>

namespace glvm::ecs::arch {
struct Archetype {
    virtual ~Archetype() = default;

    static constexpr uint32_t CAPACITY = 1024;

    uint64_t entities[CAPACITY];
    uint32_t entityCount = 0;
    uint32_t componentIds[ComponentsIndices::COMPONENTS_COUNT] = {};
    uint32_t componentCount = 0;
    void* components[ComponentsIndices::COMPONENTS_COUNT] = {};
    uint64_t mask = 0;

    uint32_t addEntity(uint64_t entity_);
    uint64_t removeEntity(uint32_t index);
};

struct EntityLocation {
    Archetype* arch;
    uint32_t index;
    static const uint8_t maxGridCellNumber = 8;
    uint8_t gridCellCounter = 0;
    Vector<float, 3> gridCellIndicies[maxGridCellNumber];
    uint32_t cellEntityIndices[maxGridCellNumber];
    // Is entity has been moved or removed.
    bool isDirty = false;
};
}; // namespace glvm::ecs::arch
