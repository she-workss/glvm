// Copyright © 2024 Maksim Manokhin a.k.a. Yuriorkis_Scream. Contacts:
// <fellfrostqtw@gmail.com> This file is part of Game Loop Versatile Modules
// (GLVM) Author: Maksim Manokhin a.k.a. Yuriorkis_Scream License:
// http://opensource.org/licenses/MIT

#include "glvm/Systems/CollisionSystem.hpp"

#include "glvm/ArchetypeECS/ArchECS_Types.hpp"
#include "glvm/ArchetypeECS/ArchECS_Utils.hpp"
#include "glvm/ArchetypeECS/ArchECS_World.hpp"
#include "glvm/ArchetypeECS/ArchetypeInterface.hpp"
#include "glvm/Archetypes/CrosshairArchetype.hpp"
#include "glvm/Archetypes/DirectionalLightArchetype.hpp"
#include "glvm/Archetypes/EnemyArchetype.hpp"
#include "glvm/Archetypes/InventoryArchetype.hpp"
#include "glvm/Archetypes/ItemArchetype.hpp"
#include "glvm/Archetypes/LevelChunkArchetype.hpp"
#include "glvm/Archetypes/PlayerArchetype.hpp"
#include "glvm/Archetypes/PointLightArchetype.hpp"
#include "glvm/Archetypes/ProjectileArchetype.hpp"
#include "glvm/Archetypes/SpotLightArchetype.hpp"
#include "glvm/Archetypes/StaticMeshArchetype.hpp"
#include "glvm/Common/CommonFunctions.hpp"
#include "glvm/Components/ColliderComponent.hpp"
#include "glvm/Components/ColliderFlagsComponent.hpp"
#include "glvm/Components/MoveComponent.hpp"
#include "glvm/Components/TransformComponent.hpp"
#include "glvm/Components/VertexComponent.hpp"
#include "glvm/Systems/ProjectileSystem.hpp"
#include "glvm/Vector.hpp"
#include "glvm/VertexMath.hpp"

#include <cstdint>
#include <sys/types.h>

namespace GLVM::ecs {
void CCollisionSystem::Update() {
    namespace arch = GLVM::ecs::arch;
    namespace cm = GLVM::ecs::components;

    /// Spatial grid common data
    const arch::SpatialGrid& spatialGrid = arch::world.spatialGrid;
    assert(
        spatialGrid.width > 0 && spatialGrid.height > 0 && spatialGrid.depth > 0
    );
    const float chunkSize = spatialGrid.grid[0][0][0].size;

    const float chunkHalfWidth = spatialGrid.width * chunkSize * 0.5f;
    const float chunkHalfHeight = spatialGrid.height * chunkSize * 0.5f;
    const float chunkHalfDepth = spatialGrid.depth * chunkSize * 0.5f;

    cachedArchetypesNumber = 0;
    arch::world.searchCacheArchetypes(
        requiredMask,
        cachedArchetypes,
        cachedArchetypesNumber
    );

    const float cameraSpeed = 5.5f * fDelta_Time_;
    /// Outer cycle on every archetype
    for (uint32_t x = 0; x < cachedArchetypesNumber; ++x) {
        arch::Archetype* arch = cachedArchetypes[x];
        view.backtrackingTransforms =
            (ecs::components::transform*)
                arch->components[arch::ComponentsIndices::TRANSFORM_COMPONENT];
        view.backtrackingColliders =
            (ecs::components::collider*)
                arch->components[arch::ComponentsIndices::COLLIDER_COMPONENT];
        view.backtrackingColliderFlags =
            (ecs::components::colliderFlags*)arch
                ->components[arch::ComponentsIndices::COLLIDER_FLAGS_COMPONENT];
        view.backtrackingMeshes =
            (ecs::components::mesh*)
                arch->components[arch::ComponentsIndices::MESH_COMPONENT];

        for (unsigned int i = 0; i < arch->entityCount; ++i) {
            /// Count on every entity in current outer archetype
            uint32_t backtrackingEntityID = arch->entities[i];

            uint8_t groudCollisionTurnOffMask =
                (1u << 0) | (0u << 1) | (1u << 2) | (1u << 3);
            if (view.backtrackingColliderFlags && view.backtrackingColliders
                && view.backtrackingMeshes && view.backtrackingTransforms) {
                view.backtrackingColliderFlags[i].flags =
                    view.backtrackingColliderFlags[i].flags
                    & groudCollisionTurnOffMask;
                view.backtrackingColliders[i].colliders.clear();
                components::mesh backtrackinEntityMesh =
                    view.backtrackingMeshes[i];
                components::MeshHandle backtrackingEntityMeshHandle =
                    backtrackinEntityMesh.handle;
                components::transform* backtrackingTransformComponent =
                    &view.backtrackingTransforms[i];
                vec3 backtrackingTransform =
                    backtrackingTransformComponent->position;
                [[maybe_unused]] float backtrackingScale =
                    backtrackingTransformComponent->scale;

                arch::componentMask moveRequiredMask =
                    (1ul << arch::ComponentsIndices::MOVE_COMPONENT);
                /// Check if outer current archetype has move component
                if (arch::matchesRequiredMask(arch->mask, moveRequiredMask)) {
                    view.backtrackingMove =
                        (ecs::components::move*)arch
                            ->components[arch::ComponentsIndices::MOVE_COMPONENT];
                    backtrackingTransform +=
                        Normalize(view.backtrackingMove[i].frameMovement)
                        * cameraSpeed;
                    backtrackingTransform += view.backtrackingMove[i].gravity;
                }

                ///< Collect entities from grid chunks
                core::MeshAxisMaxAbsoluteValues entityChunkBounds =
                    allMeshMaxAbsoluteValues[backtrackingEntityMeshHandle.id];
                core::vector<vec3> entityBoxCornerBoundPoints =
                    computeBoxCornerBoundPoints(
                        entityChunkBounds,
                        backtrackingTransformComponent->position,
                        backtrackingTransformComponent->scale
                    );

                core::vector<u32>
                    collectedEntities; ///< Result array with collected entities
                /*
                  Need only left bottom back cornder point and right upper front
                  conrner point to obtain all box bounds
                */
                const vec3 minEntityPosition = entityBoxCornerBoundPoints[0];
                const vec3 maxEntityPosition = entityBoxCornerBoundPoints[1];

                const u32 indexMinX =
                    (minEntityPosition[0] + chunkHalfWidth) / chunkSize;
                const u32 indexMinY =
                    (minEntityPosition[1] + chunkHalfHeight) / chunkSize;
                const u32 indexMinZ =
                    (minEntityPosition[2] + chunkHalfDepth) / chunkSize;

                const u32 indexMaxX =
                    (maxEntityPosition[0] + chunkHalfWidth) / chunkSize;
                const u32 indexMaxY =
                    (maxEntityPosition[1] + chunkHalfHeight) / chunkSize;
                const u32 indexMaxZ =
                    (maxEntityPosition[2] + chunkHalfDepth) / chunkSize;

                assert(
                    indexMinX <= indexMaxX && indexMinY <= indexMaxY
                    && indexMinZ <= indexMaxZ
                );
                assert(
                    indexMinX < spatialGrid.width
                    && indexMinY < spatialGrid.height
                    && indexMinZ < spatialGrid.depth
                );
                assert(
                    indexMaxX < spatialGrid.width
                    && indexMaxY < spatialGrid.height
                    && indexMaxZ < spatialGrid.depth
                );

                for (u32 i2 = indexMinZ; i2 <= indexMaxZ; ++i2) {
                    for (u32 i3 = indexMinY; i3 <= indexMaxY; ++i3) {
                        for (u32 i4 = indexMinX; i4 <= indexMaxX; ++i4) {
                            const core::vector<u32>& chunkEntities =
                                spatialGrid.grid[i2][i3][i4].entities;
                            for (u32 i5 = 0; i5 < chunkEntities.GetSize();
                                 ++i5) {
                                const u32 entity = chunkEntities[i5];
                                if (!core::isExist(collectedEntities, entity)) {
                                    collectedEntities.Push(entity);
                                }
                            }
                        }
                    }
                }

                /// Inner cycle on every archetype
                /// Count on every entity in current inner archetype
                for (unsigned int j = 0; j < collectedEntities.GetSize(); ++j) {
                    /// Check for same entityID and iteration
                    uint32_t comparedEntityID = collectedEntities[j];
                    if (backtrackingEntityID == comparedEntityID) {
                        continue;
                    }

                    arch::EntityLocation comparedEntityLocation =
                        arch::world
                            .entityLocations[arch::getId(comparedEntityID)];
                    const uint32_t comparedEntityIndex =
                        comparedEntityLocation.index;

                    components::MeshHandle comparedEntityMeshHandle;
                    if (arch::matchesRequiredMask(
                            comparedEntityLocation.arch->mask,
                            requiredMask
                        )) {
                        arch::Archetype* arch = comparedEntityLocation.arch;
                        view.comparedTransforms = &(
                            (ecs::components::transform*)arch->components
                                [arch::ComponentsIndices::TRANSFORM_COMPONENT]
                        )[comparedEntityIndex];
                        view.comparedMeshes = &(
                            (ecs::components::mesh*)arch->components
                                [arch::ComponentsIndices::MESH_COMPONENT]
                        )[comparedEntityIndex];
                        comparedEntityMeshHandle = view.comparedMeshes->handle;

                        arch::componentMask moveRequiredMask =
                            (1ul << arch::ComponentsIndices::MOVE_COMPONENT);
                        if (arch::matchesRequiredMask(
                                comparedEntityLocation.arch->mask,
                                moveRequiredMask
                            )) {
                            view.comparedMove = &(
                                (ecs::components::move*)arch->components
                                    [arch::ComponentsIndices::MOVE_COMPONENT]
                            )[comparedEntityIndex];
                        }
                    }

                    components::transform* comparedTransformComponent =
                        view.comparedTransforms;
                    components::move* comparedMoveComponent = view.comparedMove;

                    vec3 comparedTransform = vec3(0.0f, 0.0f, 0.0f);
                    float comparedScale = 0.0f;
                    comparedTransform = comparedTransformComponent->position;
                    comparedScale = comparedTransformComponent->scale;

                    vec3 gravityTest {};
                    if (comparedMoveComponent != nullptr) {
                        comparedTransform +=
                            Normalize(comparedMoveComponent->frameMovement)
                            * cameraSpeed;
                        comparedTransform += comparedMoveComponent->gravity;
                        gravityTest = comparedMoveComponent->gravity;
                    }

                    bool boxColliderFlag = false;
                    bool upperActorCheckFlag = false;

                    core::MeshAxisMaxAbsoluteValues
                        backtrackingMeshAxisMaxAbsoluteValues =
                            allMeshMaxAbsoluteValues[backtrackingEntityMeshHandle
                                                         .id];
                    core::MeshAxisMaxAbsoluteValues
                        comparedMeshAxisMaxAbsoluteValues = {};
                    if (comparedEntityMeshHandle.id
                        < allMeshMaxAbsoluteValues.GetSize()) {
                        comparedMeshAxisMaxAbsoluteValues =
                            allMeshMaxAbsoluteValues[comparedEntityMeshHandle.id];
                    }

                    boxColliderFlag = core::BoxCollider(
                        backtrackingTransform,
                        comparedTransform,
                        backtrackingScale,
                        comparedScale,
                        backtrackingMeshAxisMaxAbsoluteValues,
                        comparedMeshAxisMaxAbsoluteValues
                    );

                    if (boxColliderFlag) {
                        upperActorCheckFlag = UpperActorCheck(
                            backtrackingTransform,
                            comparedTransform,
                            backtrackingScale,
                            comparedScale,
                            backtrackingEntityMeshHandle,
                            comparedEntityMeshHandle
                        );
                    }

                    if (upperActorCheckFlag && boxColliderFlag) {
                        uint8_t groudCollisionTurnOnMask =
                            (0u << 0) | (1u << 1) | (0u << 2) | (0u << 3);
                        view.backtrackingColliderFlags[i].flags =
                            view.backtrackingColliderFlags[i].flags
                            | groudCollisionTurnOnMask;
                        view.backtrackingColliders[i].colliders.Push(
                            comparedEntityID
                        );

                        continue;
                    }

                    if (boxColliderFlag) {
                        uint8_t wallCollisionTurnOnMask =
                            (1u << 0) | (0u << 1) | (0u << 2) | (0u << 3);
                        view.backtrackingColliderFlags[i].flags =
                            view.backtrackingColliderFlags[i].flags
                            | wallCollisionTurnOnMask;
                        view.backtrackingColliders[i].colliders.Push(
                            comparedEntityID
                        );

                        continue;
                    }
                }
            }
        }
    }
    cachedArchetypesNumber = 0;
}

bool CCollisionSystem::UpperActorCheck(
    vec3 backtrackingPosition,
    vec3 comparedPosition,
    float backtrackingScale,
    float comparedScale,
    components::MeshHandle backtrackingMeshHandle,
    components::MeshHandle comparedMeshHandle
) {
    core::MeshAxisMaxAbsoluteValues backtrackingMeshAxisMaxAbsoluteValues =
        allMeshMaxAbsoluteValues[backtrackingMeshHandle.id];

    // std::cout << "array size: " << allMeshMaxAbsoluteValues.GetSize() <<
    // std::endl; std::cout << "mesh id: " << comparedMeshHandle.id << std::endl;

    core::MeshAxisMaxAbsoluteValues comparedMeshAxisMaxAbsoluteValues =
        allMeshMaxAbsoluteValues[comparedMeshHandle.id];

    constexpr float epsilon = 0.15f;
    if (backtrackingPosition[1]
            + backtrackingMeshAxisMaxAbsoluteValues.origin_offset_y
            - backtrackingMeshAxisMaxAbsoluteValues.absolute_y
                * backtrackingScale
            + epsilon
        > comparedPosition[1]
            + comparedMeshAxisMaxAbsoluteValues.origin_offset_y
            + comparedMeshAxisMaxAbsoluteValues.absolute_y * comparedScale) {
        return true;
    }

    return false;
}
} // namespace GLVM::ecs
