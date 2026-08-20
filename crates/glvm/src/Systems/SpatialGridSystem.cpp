#include "glvm/Systems/SpatialGridSystem.hpp"

#include "glvm/ArchetypeECS/ArchECS_World.hpp"
#include "glvm/Common/CommonFunctions.hpp"
#include "glvm/Vector.hpp"

#include <algorithm>

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

        for (u32 i1 = 0; i1 < arch->entityCount; ++i1) {
            const arch::entity entity = arch->entities[i1];
            ecs::arch::EntityLocation& entityLocation =
                ecs::arch::world.entityLocations[ecs::arch::getId(entity)];
            if (!entityLocation.isDirty && isInitialized) {
                continue;
            }

            if (entityLocation.gridCellCounter > 0) {
                for (u32 i2 = 0; i2 < entityLocation.gridCellCounter; ++i2) {
                    u32 z = entityLocation.gridCellIndicies[i2][0];
                    u32 y = entityLocation.gridCellIndicies[i2][1];
                    u32 x = entityLocation.gridCellIndicies[i2][2];
                    core::vector<u32>& chunkEntities =
                        spatialGrid.grid[z][y][x].entities;
                    // Remove by value: the recorded index can be stale after
                    // other removals shifted the cell's vector.
                    for (u32 i3 = 0; i3 < chunkEntities.GetSize(); ++i3) {
                        if (chunkEntities[i3] == entity) {
                            chunkEntities.Remove(i3);
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
            core::vector<vec3> entityBoxCornerBoundPoints =
                computeBoxCornerBoundPoints(
                    entityChunkBounds,
                    transform.position,
                    transform.scale
                );

            // Need only left bottom back corner point and right upper front
            // corner point to obtain all box bounds.
            const vec3 minEntityPosition = entityBoxCornerBoundPoints[0];
            const vec3 maxEntityPosition = entityBoxCornerBoundPoints[1];

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

            for (u32 i2 = indexMinZ; i2 <= indexMaxZ; ++i2) {
                for (u32 i3 = indexMinY; i3 <= indexMaxY; ++i3) {
                    for (u32 i4 = indexMinX; i4 <= indexMaxX; ++i4) {
                        core::vector<u32>& chunkEntities =
                            spatialGrid.grid[i2][i3][i4].entities;
                        if (!core::isExist<u32>(chunkEntities, entity)) {
                            chunkEntities.Push(entity);
                            const u32 currentGridCell =
                                entityLocation.gridCellCounter;
                            assert(
                                currentGridCell < 8
                            ); ///< 8 is a maximum number for 1 entity to exist
                               ///< in grid cell
                            entityLocation.gridCellIndicies[currentGridCell] =
                                vec3(i2, i3, i4);
                            entityLocation.cellEntityIndices[currentGridCell] =
                                chunkEntities.GetSize() - 1;
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
