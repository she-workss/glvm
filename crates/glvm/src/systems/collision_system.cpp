#include "glvm/systems/collision_system.hpp"

#include "glvm/archetype_ecs/arch_ecs_types.hpp"
#include "glvm/archetype_ecs/arch_ecs_utils.hpp"
#include "glvm/archetype_ecs/arch_ecs_world.hpp"
#include "glvm/archetype_ecs/archetype_interface.hpp"
#include "glvm/archetypes/crosshair_archetype.hpp"
#include "glvm/archetypes/directional_light_archetype.hpp"
#include "glvm/archetypes/enemy_archetype.hpp"
#include "glvm/archetypes/inventory_archetype.hpp"
#include "glvm/archetypes/item_archetype.hpp"
#include "glvm/archetypes/level_chunk_archetype.hpp"
#include "glvm/archetypes/player_archetype.hpp"
#include "glvm/archetypes/point_light_archetype.hpp"
#include "glvm/archetypes/projectile_archetype.hpp"
#include "glvm/archetypes/spot_light_archetype.hpp"
#include "glvm/archetypes/static_mesh_archetype.hpp"
#include "glvm/common/common_functions.hpp"
#include "glvm/components/collider_component.hpp"
#include "glvm/components/collider_flags_component.hpp"
#include "glvm/components/move_component.hpp"
#include "glvm/components/transform_component.hpp"
#include "glvm/components/vertex_component.hpp"
#include "glvm/systems/projectile_system.hpp"
#include "glvm/vertex_math.hpp"

#include <algorithm>
#include <cstdint>
#include <sys/types.h>
#include <vector>

namespace glvm::ecs {
void CCollisionSystem::Update() {
    namespace arch = glvm::ecs::arch;

    // Spatial grid common data.
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
    // Outer cycle on every archetype.
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
            // Count on every entity in current outer archetype.
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
                Vector<float, 3> backtrackingTransform =
                    backtrackingTransformComponent->position;
                [[maybe_unused]] float backtrackingScale =
                    backtrackingTransformComponent->scale;

                uint64_t moveRequiredMask =
                    (1ul << arch::ComponentsIndices::MOVE_COMPONENT);
                // Check if outer current archetype has move component.
                if (arch::matchesRequiredMask(arch->mask, moveRequiredMask)) {
                    view.backtrackingMove =
                        (ecs::components::move*)arch
                            ->components[arch::ComponentsIndices::MOVE_COMPONENT];
                    backtrackingTransform +=
                        Normalize(view.backtrackingMove[i].frameMovement)
                        * cameraSpeed;
                    backtrackingTransform += view.backtrackingMove[i].gravity;
                }

                // Collect entities from grid chunks.
                core::MeshAxisMaxAbsoluteValues entityChunkBounds =
                    allMeshMaxAbsoluteValues[backtrackingEntityMeshHandle.id];
                std::vector<Vector<float, 3>> entityBoxCornerBoundPoints =
                    computeBoxCornerBoundPoints(
                        entityChunkBounds,
                        backtrackingTransformComponent->position,
                        backtrackingTransformComponent->scale
                    );

                // Result array with collected entities.
                std::vector<uint32_t> collectedEntities;
                // Need only left bottom back corner point and right upper front
                // corner point to obtain all box bounds
                const Vector<float, 3> minEntityPosition = entityBoxCornerBoundPoints[0];
                const Vector<float, 3> maxEntityPosition = entityBoxCornerBoundPoints[1];

                int indexMinX =
                    (int)((minEntityPosition[0] + chunkHalfWidth) / chunkSize);
                int indexMinY =
                    (int)((minEntityPosition[1] + chunkHalfHeight) / chunkSize);
                int indexMinZ =
                    (int)((minEntityPosition[2] + chunkHalfDepth) / chunkSize);

                int indexMaxX =
                    (int)((maxEntityPosition[0] + chunkHalfWidth) / chunkSize);
                int indexMaxY =
                    (int)((maxEntityPosition[1] + chunkHalfHeight) / chunkSize);
                int indexMaxZ =
                    (int)((maxEntityPosition[2] + chunkHalfDepth) / chunkSize);

                // Entity can legitimately leave the fixed-size world grid -
                // clamp to nearest edge cell instead of crashing.
                indexMinX =
                    std::clamp(indexMinX, 0, (int)spatialGrid.width - 1);
                indexMinY =
                    std::clamp(indexMinY, 0, (int)spatialGrid.height - 1);
                indexMinZ =
                    std::clamp(indexMinZ, 0, (int)spatialGrid.depth - 1);
                indexMaxX =
                    std::clamp(indexMaxX, 0, (int)spatialGrid.width - 1);
                indexMaxY =
                    std::clamp(indexMaxY, 0, (int)spatialGrid.height - 1);
                indexMaxZ =
                    std::clamp(indexMaxZ, 0, (int)spatialGrid.depth - 1);

                for (uint32_t i2 = indexMinZ; i2 <= indexMaxZ; ++i2) {
                    for (uint32_t i3 = indexMinY; i3 <= indexMaxY; ++i3) {
                        for (uint32_t i4 = indexMinX; i4 <= indexMaxX; ++i4) {
                            const std::vector<uint32_t>& chunkEntities =
                                spatialGrid.grid[i2][i3][i4].entities;
                            for (uint32_t i5 = 0; i5 < chunkEntities.size(); ++i5) {
                                const uint32_t entity = chunkEntities[i5];
                                if (!core::isExist(collectedEntities, entity)) {
                                    collectedEntities.push_back(entity);
                                }
                            }
                        }
                    }
                }

                // Inner cycle on every archetype.
                // Count on every entity in current inner archetype.
                for (unsigned int j = 0; j < collectedEntities.size(); ++j) {
                    // Check for same entityID and iteration.
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
                    if (comparedEntityLocation.arch == nullptr) {
                        // Entity was removed from the world but a stale
                        // reference survived in the grid, skip it.
                        continue;
                    }
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

                        uint64_t moveRequiredMask =
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

                    Vector<float, 3> comparedTransform = Vector<float, 3>(0.0f, 0.0f, 0.0f);
                    float comparedScale = 0.0f;
                    comparedTransform = comparedTransformComponent->position;
                    comparedScale = comparedTransformComponent->scale;

                    Vector<float, 3> gravityTest {};
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
                        < allMeshMaxAbsoluteValues.size()) {
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
                        view.backtrackingColliders[i].colliders.push_back(
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
                        view.backtrackingColliders[i].colliders.push_back(
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
    Vector<float, 3> backtrackingPosition,
    Vector<float, 3> comparedPosition,
    float backtrackingScale,
    float comparedScale,
    components::MeshHandle backtrackingMeshHandle,
    components::MeshHandle comparedMeshHandle
) {
    core::MeshAxisMaxAbsoluteValues backtrackingMeshAxisMaxAbsoluteValues =
        allMeshMaxAbsoluteValues[backtrackingMeshHandle.id];

    core::MeshAxisMaxAbsoluteValues comparedMeshAxisMaxAbsoluteValues =
        allMeshMaxAbsoluteValues[comparedMeshHandle.id];

    constexpr float epsilon = 0.15f;
    if (backtrackingPosition[1]
            + backtrackingMeshAxisMaxAbsoluteValues.origin_offset_y
                * backtrackingScale
            - backtrackingMeshAxisMaxAbsoluteValues.absolute_y
                * backtrackingScale
            + epsilon
        > comparedPosition[1]
            + comparedMeshAxisMaxAbsoluteValues.origin_offset_y * comparedScale
            + comparedMeshAxisMaxAbsoluteValues.absolute_y * comparedScale) {
        return true;
    }

    return false;
}
} // namespace glvm::ecs
