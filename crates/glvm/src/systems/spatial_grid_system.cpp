#include "glvm/systems/spatial_grid_system.hpp"

#include "glvm/archetype_ecs/arch_ecs_world.hpp"
#include "glvm/common/common_functions.hpp"

#include <algorithm>
#include <vector>

namespace glvm::ecs {
void SpatialGridSystem::Update() {
    namespace arch = glvm::ecs::arch;

    arch::SpatialGrid& spatialGrid = arch::world.spatialGrid;
    assert(
        spatialGrid.width > 0 && spatialGrid.height > 0 && spatialGrid.depth > 0
    );
    const float chunkSize = spatialGrid.grid[0][0][0].size;

    const float halfWidth = spatialGrid.width * chunkSize * 0.5f;
    const float halfHeight = spatialGrid.height * chunkSize * 0.5f;
    const float halfDepth = spatialGrid.depth * chunkSize * 0.5f;

    cachedArchetypesNumber = 0;
    arch::world.searchCacheArchetypes(
        requiredMask,
        cachedArchetypes,
        cachedArchetypesNumber
    );

    for (uint32_t i0 = 0; i0 < cachedArchetypesNumber; ++i0) {
        arch::Archetype* arch = cachedArchetypes[i0];
        view.transforms =
            (ecs::components::transform*)
                arch->components[arch::ComponentsIndices::TRANSFORM_COMPONENT];
        view.meshes =
            (ecs::components::mesh*)
                arch->components[arch::ComponentsIndices::MESH_COMPONENT];

        for (uint32_t i1 = 0; i1 < arch->entityCount; ++i1) {
            const uint64_t entity = arch->entities[i1];
            ecs::arch::EntityLocation& entityLocation =
                ecs::arch::world.entityLocations[ecs::arch::getId(entity)];
            if (!entityLocation.isDirty && isInitialized) {
                continue;
            }

            if (entityLocation.gridCellCounter > 0) {
                for (uint32_t i2 = 0; i2 < entityLocation.gridCellCounter; ++i2) {
                    uint32_t z = entityLocation.gridCellIndicies[i2][0];
                    uint32_t y = entityLocation.gridCellIndicies[i2][1];
                    uint32_t x = entityLocation.gridCellIndicies[i2][2];
                    std::vector<uint32_t>& chunkEntities =
                        spatialGrid.grid[z][y][x].entities;
                    // Remove by value: the recorded index can be stale after
                    // other removals shifted the cell's vector.
                    for (uint32_t i3 = 0; i3 < chunkEntities.size(); ++i3) {
                        if (chunkEntities[i3] == entity) {
                            chunkEntities.erase(chunkEntities.begin() + i3);
                            break;
                        }
                    }
                }
                entityLocation.gridCellCounter = 0;
            }

            const components::transform& transform = view.transforms[i1];
            const components::mesh& mesh = view.meshes[i1];

            components::MeshHandle entityMeshHandle = mesh.handle;
            core::MeshAxisMaxAbsoluteValues entityChunkBounds =
                allMeshMaxAbsoluteValues[entityMeshHandle.id];
            std::vector<Vector<float, 3>> entityBoxCornerBoundPoints =
                computeBoxCornerBoundPoints(
                    entityChunkBounds,
                    transform.position,
                    transform.scale
                );

            // Need only left bottom back corner point and right upper front
            // corner point to obtain all box bounds.
            const Vector<float, 3> minEntityPosition = entityBoxCornerBoundPoints[0];
            const Vector<float, 3> maxEntityPosition = entityBoxCornerBoundPoints[1];

            int indexMinX =
                (int)((minEntityPosition[0] + halfWidth) / chunkSize);
            int indexMinY =
                (int)((minEntityPosition[1] + halfHeight) / chunkSize);
            int indexMinZ =
                (int)((minEntityPosition[2] + halfDepth) / chunkSize);

            int indexMaxX =
                (int)((maxEntityPosition[0] + halfWidth) / chunkSize);
            int indexMaxY =
                (int)((maxEntityPosition[1] + halfHeight) / chunkSize);
            int indexMaxZ =
                (int)((maxEntityPosition[2] + halfDepth) / chunkSize);

            // Entity can legitimately leave the fixed-size world grid (fell off
            // the world edge, projectile flew away) - clamp to nearest edge
            // cell instead of crashing.
            indexMinX = std::clamp(indexMinX, 0, (int)spatialGrid.width - 1);
            indexMinY = std::clamp(indexMinY, 0, (int)spatialGrid.height - 1);
            indexMinZ = std::clamp(indexMinZ, 0, (int)spatialGrid.depth - 1);
            indexMaxX = std::clamp(indexMaxX, 0, (int)spatialGrid.width - 1);
            indexMaxY = std::clamp(indexMaxY, 0, (int)spatialGrid.height - 1);
            indexMaxZ = std::clamp(indexMaxZ, 0, (int)spatialGrid.depth - 1);

            for (uint32_t i2 = indexMinZ; i2 <= indexMaxZ; ++i2) {
                for (uint32_t i3 = indexMinY; i3 <= indexMaxY; ++i3) {
                    for (uint32_t i4 = indexMinX; i4 <= indexMaxX; ++i4) {
                        std::vector<uint32_t>& chunkEntities =
                            spatialGrid.grid[i2][i3][i4].entities;
                        if (!core::isExist<uint32_t>(chunkEntities, entity)) {
                            chunkEntities.push_back(entity);
                            const uint32_t currentGridCell =
                                entityLocation.gridCellCounter;
                            assert(
                                currentGridCell < 8
                            ); ///< 8 is a maximum number for 1 entity to exist
                               ///< in grid cell
                            entityLocation.gridCellIndicies[currentGridCell] =
                                Vector<float, 3>(i2, i3, i4);
                            entityLocation.cellEntityIndices[currentGridCell] =
                                chunkEntities.size() - 1;
                            entityLocation.isDirty = false;
                            ++entityLocation.gridCellCounter;
                        }
                    }
                }
            }
        }
    }
    if (!isInitialized) {
        isInitialized = true;
    }
}

}; // namespace glvm::ecs
