#pragma once

#include "glvm/archetype_ecs/arch_ecs_utils.hpp"
#include "glvm/archetype_ecs/archetype_interface.hpp"
#include "glvm/typenames.hpp"

#include <vector>

namespace glvm::ecs::arch {
struct GridChunk {
    Vector<float, 3> position;
    static constexpr float size = 32;
    std::vector<uint32_t> entities;
};

struct SpatialGrid {
    static const uint32_t width = 64;
    static const uint32_t height = 64;
    static const uint32_t depth = 64;
    GridChunk grid[width][height][depth];
};

struct World {
    World();
    ~World();

    SpatialGrid spatialGrid;
    std::vector<Archetype*> archetypes;
    std::vector<EntityLocation> entityLocations;

    void addEntityToArchetype(uint64_t entity_, Archetype* arch);
    void removeEntity(uint64_t entity_);
    void searchCacheArchetypes(
        uint64_t requiredMask,
        arch::Archetype* cachedArchetypes[],
        uint32_t& cachedArchetypesNumber
    );
};

extern World world;
}; // namespace glvm::ecs::arch
