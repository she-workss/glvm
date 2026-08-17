// This file is part of Game Loop Versatile Modules (GLVM)
// Copyright © 2024 Maksim Manokhin a.k.a. Yuriorkis_Scream. Contacts:
// <fellfrostqtw@gmail.com> Author: Maksim Manokhin a.k.a. Yuriorkis_Scream
// License: http://opensource.org/licenses/MIT

#include "glvm/Systems/SpatialGridSystem.hpp"

#include "glvm/ArchetypeECS/ArchECS_World.hpp"
#include "glvm/Common/CommonFunctions.hpp"
#include "glvm/Vector.hpp"

namespace GLVM::ecs {
void SpatialGridSystem::Update() {
    namespace arch = GLVM::ecs::arch;

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
                for (u32 i2 = 0; i2 < entityLocation.maxGridCellNumber; ++i2) {
                    u32 z = entityLocation.gridCellIndicies[i2][0];
                    u32 y = entityLocation.gridCellIndicies[i2][1];
                    u32 x = entityLocation.gridCellIndicies[i2][2];
                    u32 cellEntityIndex = entityLocation.cellEntityIndices[i2];
                    spatialGrid.grid[z][y][x].entities.Remove(cellEntityIndex);
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

            /*
              Need only left bottom back cornder point and right upper front
              conrner point to obtain all box bounds
            */
            const vec3 minEntityPosition = entityBoxCornerBoundPoints[0];
            const vec3 maxEntityPosition = entityBoxCornerBoundPoints[1];

            const u32 indexMinX =
                (minEntityPosition[0] + halfWidth) / chunkSize;
            const u32 indexMinY =
                (minEntityPosition[1] + halfHeight) / chunkSize;
            const u32 indexMinZ =
                (minEntityPosition[2] + halfDepth) / chunkSize;

            const u32 indexMaxX =
                (maxEntityPosition[0] + halfWidth) / chunkSize;
            const u32 indexMaxY =
                (maxEntityPosition[1] + halfHeight) / chunkSize;
            const u32 indexMaxZ =
                (maxEntityPosition[2] + halfDepth) / chunkSize;

            assert(
                indexMinX <= indexMaxX && indexMinY <= indexMaxY
                && indexMinZ <= indexMaxZ
            );
            assert(
                indexMinX < spatialGrid.width && indexMinY < spatialGrid.height
                && indexMinZ < spatialGrid.depth
            );
            assert(
                indexMaxX < spatialGrid.width && indexMaxY < spatialGrid.height
                && indexMaxZ < spatialGrid.depth
            );

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

}; // namespace GLVM::ecs
