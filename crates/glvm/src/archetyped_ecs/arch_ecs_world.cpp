#include "glvm/archetype_ecs/arch_ecs_world.hpp"

#include "glvm/archetypes/crosshair_archetype.hpp"
#include "glvm/archetypes/directional_light_archetype.hpp"
#include "glvm/archetypes/inventory_archetype.hpp"
#include "glvm/archetypes/item_archetype.hpp"
#include "glvm/archetypes/level_chunk_archetype.hpp"
#include "glvm/archetypes/point_light_archetype.hpp"
#include "glvm/archetypes/projectile_archetype.hpp"
#include "glvm/archetypes/rigid_body_archetype.hpp"
#include "glvm/archetypes/spot_light_archetype.hpp"
#include "glvm/archetypes/static_mesh_archetype.hpp"

namespace glvm::ecs::arch {
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
    const Vector<float, 3> pivot = Vector<float, 3>(
        -halfWorldWidth + halfChunkSize,
        -halfWorldHeight + halfChunkSize,
        -halfWorldDepth + halfChunkSize
    );
    for (uint32_t i0 = 0; i0 < spatialGrid.depth; ++i0) {
        for (uint32_t i1 = 0; i1 < spatialGrid.height; ++i1) {
            for (uint32_t i2 = 0; i2 < spatialGrid.width; ++i2) {
                spatialGrid.grid[i0][i1][i2].position =
                    Vector<float, 3>(i2 * chunkSize, i1 * chunkSize, i0 * chunkSize)
                    + pivot;
            }
        }
    }
}

World::~World() {
    for (unsigned int i = 0; i < archetypes.size(); ++i) {
        delete archetypes[i];
        archetypes[i] = nullptr;
    }
}

void World::addEntityToArchetype(uint64_t entity_, Archetype* arch) {
    uint32_t id_ = getId(entity_);

    if (id_ >= entityLocations.size()) {
        entityLocations.resize(id_ + 1);
    }

    EntityLocation& location = entityLocations[id_];

    if (location.arch != nullptr) {
        assert(false && "unsigned int already assigned to archetype");
    }

    uint32_t index = arch->addEntity(entity_);

    location.arch = arch;
    location.index = index;
}

void World::removeEntity(uint64_t entity_) {
    uint32_t id_ = getId(entity_);
    EntityLocation& location = entityLocations[id_];
    // Remove entity from spatial grid cells it occupies, otherwise stale
    // references crash collision/physics on later frames.
    if (location.gridCellCounter > 0) {
        for (uint8_t i = 0; i < location.gridCellCounter; ++i) {
            uint32_t z = location.gridCellIndicies[i][0];
            uint32_t y = location.gridCellIndicies[i][1];
            uint32_t x = location.gridCellIndicies[i][2];
            std::vector<uint32_t>& chunkEntities =
                spatialGrid.grid[z][y][x].entities;
            for (uint32_t k = 0; k < chunkEntities.size(); ++k) {
                if (chunkEntities[k] == entity_) {
                    chunkEntities.erase(chunkEntities.begin() + k);
                    break;
                }
            }
        }
        location.gridCellCounter = 0;
    }

    Archetype* arch = location.arch;
    uint32_t index = location.index;

    uint64_t moved = arch->removeEntity(index);

    if (moved != entity_) {
        uint32_t movedId = getId(moved);
        entityLocations[movedId].index = index;
        entityLocations[movedId].arch = arch;
    }
    std::cout << "remove entity with id: " << id_ << std::endl;
    location.arch = nullptr;
}

void World::searchCacheArchetypes(
    uint64_t requiredMask,
    arch::Archetype* cachedArchetypes[],
    uint32_t& cachedArchetypesNumber
) {
    for (uint32_t i = 0; i < arch::world.archetypes.size(); ++i) {
        arch::Archetype* arch = arch::world.archetypes[i];

        if ((arch->mask & requiredMask) == requiredMask) {
            cachedArchetypes[cachedArchetypesNumber] = arch;
            ++cachedArchetypesNumber;
        }
    }
}
}; // namespace glvm::ecs::arch
