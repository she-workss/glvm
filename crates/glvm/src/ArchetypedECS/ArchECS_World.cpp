#include "glvm/ArchetypeECS/ArchECS_World.hpp"

#include "glvm/Archetypes/CrosshairArchetype.hpp"
#include "glvm/Archetypes/DirectionalLightArchetype.hpp"
#include "glvm/Archetypes/InventoryArchetype.hpp"
#include "glvm/Archetypes/ItemArchetype.hpp"
#include "glvm/Archetypes/LevelChunkArchetype.hpp"
#include "glvm/Archetypes/PointLightArchetype.hpp"
#include "glvm/Archetypes/ProjectileArchetype.hpp"
#include "glvm/Archetypes/RigidBodyArchetype.hpp"
#include "glvm/Archetypes/SpotLightArchetype.hpp"
#include "glvm/Archetypes/StaticMeshArchetype.hpp"

namespace GLVM::ecs::arch {
World world = {};

World::World() {
    assert(
        spatialGrid.width > 0 && spatialGrid.height > 0 && spatialGrid.depth > 0
    );

    const float chunkSize = spatialGrid.grid[0][0][0].size;
    const float halfWorldWidth = spatialGrid.width * chunkSize * 0.5f;
    const float halfWorldHeight = spatialGrid.height * chunkSize * 0.5f;
    const float halfWorldDepth = spatialGrid.depth * chunkSize * 0.5f;
    const float halfChunkSize = chunkSize * 0.5f;
    const vec3 pivot = vec3(
        -halfWorldWidth + halfChunkSize,
        -halfWorldHeight + halfChunkSize,
        -halfWorldDepth + halfChunkSize
    );
    for (u32 i0 = 0; i0 < spatialGrid.depth; ++i0) {
        for (u32 i1 = 0; i1 < spatialGrid.height; ++i1) {
            for (u32 i2 = 0; i2 < spatialGrid.width; ++i2) {
                spatialGrid.grid[i0][i1][i2].position =
                    vec3(i2 * chunkSize, i1 * chunkSize, i0 * chunkSize)
                    + pivot;
            }
        }
    }
}

World::~World() {
    for (unsigned int i = 0; i < archetypes.GetSize(); ++i) {
        delete archetypes[i];
        archetypes[i] = nullptr;
    }
}

void World::addEntityToArchetype(entity entity_, Archetype* arch) {
    id id_ = getId(entity_);

    if (id_ >= entityLocations.GetSize()) {
        entityLocations.Resize(id_ + 1);
    }

    EntityLocation& location = entityLocations[id_];

    if (location.arch != nullptr) {
        assert(false && "Entity already assigned to archetype");
    }

    uint32_t index = arch->addEntity(entity_);

    location.arch = arch;
    location.index = index;
}

void World::removeEntity(entity entity_) {
    id id_ = getId(entity_);
    EntityLocation& location = entityLocations[id_];

    /// Remove entity from spatial grid cells it occupies, otherwise stale
    /// references crash collision/physics on later frames.
    if (location.gridCellCounter > 0) {
        for (u8 i = 0; i < location.gridCellCounter; ++i) {
            u32 z = location.gridCellIndicies[i][0];
            u32 y = location.gridCellIndicies[i][1];
            u32 x = location.gridCellIndicies[i][2];
            core::vector<u32>& chunkEntities =
                spatialGrid.grid[z][y][x].entities;
            for (u32 k = 0; k < chunkEntities.GetSize(); ++k) {
                if (chunkEntities[k] == entity_) {
                    chunkEntities.Remove(k);
                    break;
                }
            }
        }
        location.gridCellCounter = 0;
    }

    Archetype* arch = location.arch;
    uint32_t index = location.index;

    entity moved = arch->removeEntity(index);

    if (moved != entity_) {
        id movedId = getId(moved);
        entityLocations[movedId].index = index;
        entityLocations[movedId].arch = arch;
    }
    std::cout << "remove entity with id: " << id_ << std::endl;
    location.arch = nullptr;
}

void World::searchCacheArchetypes(
    arch::componentMask requiredMask,
    arch::Archetype* cachedArchetypes[],
    uint32_t& cachedArchetypesNumber
) {
    for (uint32_t i = 0; i < arch::world.archetypes.GetSize(); ++i) {
        arch::Archetype* arch = arch::world.archetypes[i];

        if ((arch->mask & requiredMask) == requiredMask) {
            cachedArchetypes[cachedArchetypesNumber] = arch;
            ++cachedArchetypesNumber;
        }
    }
}
}; // namespace GLVM::ecs::arch
