#pragma once

#include "glvm/ArchetypeECS/ArchECS_Utils.hpp"
#include "glvm/ArchetypeECS/ArchetypeInterface.hpp"
#include "glvm/Vector.hpp"
#include "glvm/typenames.hpp"

namespace glvm::ecs::arch {
struct GridChunk {
    vec3 position;
    static constexpr float size = 32;
    core::vector<u32> entities;
};

struct SpatialGrid {
    static const u32 width = 64;
    static const u32 height = 64;
    static const u32 depth = 64;
    GridChunk grid[width][height][depth];
};

struct World {
    World();
    ~World();

    SpatialGrid spatialGrid;
    core::vector<Archetype*> archetypes;
    core::vector<EntityLocation> entityLocations;

    void addEntityToArchetype(entity entity_, Archetype* arch);
    void removeEntity(entity entity_);
    void searchCacheArchetypes(
        arch::componentMask requiredMask,
        arch::Archetype* cachedArchetypes[],
        uint32_t& cachedArchetypesNumber
    );
};

extern World world;
}; // namespace glvm::ecs::arch
