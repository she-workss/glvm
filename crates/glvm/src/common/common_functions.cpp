#include "glvm/common/common_functions.hpp"

#include "glvm/components/collider_component.hpp"

namespace glvm::core {
bool BoxCollider(
    const Vector<float, 3> backtrackingPos,
    const Vector<float, 3> comparedPos,
    const float backtrackingScale,
    const float comparedScale,
    const core::MeshAxisMaxAbsoluteValues& backtrackingMeshAxisMaxAbsoluteValues,
    const core::MeshAxisMaxAbsoluteValues& comparedMeshAxisMaxAbsoluteValues
) {
    return backtrackingPos[0]
            + backtrackingMeshAxisMaxAbsoluteValues.origin_offset_x
                * backtrackingScale
            + (backtrackingMeshAxisMaxAbsoluteValues.absolute_x
               * backtrackingScale)
        > comparedPos[0]
            + comparedMeshAxisMaxAbsoluteValues.origin_offset_x * comparedScale
            - (comparedMeshAxisMaxAbsoluteValues.absolute_x * comparedScale)
        && backtrackingPos[0]
            + backtrackingMeshAxisMaxAbsoluteValues.origin_offset_x
                * backtrackingScale
            - (backtrackingMeshAxisMaxAbsoluteValues.absolute_x
               * backtrackingScale)
        < comparedPos[0]
            + comparedMeshAxisMaxAbsoluteValues.origin_offset_x * comparedScale
            + (comparedMeshAxisMaxAbsoluteValues.absolute_x * comparedScale)
        && backtrackingPos[1]
            + backtrackingMeshAxisMaxAbsoluteValues.origin_offset_y
                * backtrackingScale
            + (backtrackingMeshAxisMaxAbsoluteValues.absolute_y
               * backtrackingScale)
        > comparedPos[1]
            + comparedMeshAxisMaxAbsoluteValues.origin_offset_y * comparedScale
            - (comparedMeshAxisMaxAbsoluteValues.absolute_y * comparedScale)
        && backtrackingPos[1]
            + backtrackingMeshAxisMaxAbsoluteValues.origin_offset_y
                * backtrackingScale
            - (backtrackingMeshAxisMaxAbsoluteValues.absolute_y
               * backtrackingScale)
        < comparedPos[1]
            + comparedMeshAxisMaxAbsoluteValues.origin_offset_y * comparedScale
            + (comparedMeshAxisMaxAbsoluteValues.absolute_y * comparedScale)
        && backtrackingPos[2]
            + backtrackingMeshAxisMaxAbsoluteValues.origin_offset_z
                * backtrackingScale
            + (backtrackingMeshAxisMaxAbsoluteValues.absolute_z
               * backtrackingScale)
        > comparedPos[2]
            + comparedMeshAxisMaxAbsoluteValues.origin_offset_z * comparedScale
            - (comparedMeshAxisMaxAbsoluteValues.absolute_z * comparedScale)
        && backtrackingPos[2]
            + backtrackingMeshAxisMaxAbsoluteValues.origin_offset_z
                * backtrackingScale
            - (backtrackingMeshAxisMaxAbsoluteValues.absolute_z
               * backtrackingScale)
        < comparedPos[2]
            + comparedMeshAxisMaxAbsoluteValues.origin_offset_z * comparedScale
            + (comparedMeshAxisMaxAbsoluteValues.absolute_z * comparedScale);
}

std::vector<Vector<float, 3>> computeBoxCornerBoundPoints(
    const core::MeshAxisMaxAbsoluteValues entityChunkBounds,
    Vector<float, 3> entityPosition,
    const float scale
) {
    const float halfWidht = entityChunkBounds.absolute_x * scale;
    const float halfHeight = entityChunkBounds.absolute_y * scale;
    const float halfDepth = entityChunkBounds.absolute_z * scale;
    const Vector<float, 3> centerOffset = {
        entityChunkBounds.origin_offset_x * scale,
        entityChunkBounds.origin_offset_y * scale,
        entityChunkBounds.origin_offset_z * scale
    };
    std::vector<Vector<float, 3>> result;
    // Left bottom back.
    result.push_back(
        entityPosition + centerOffset
        + Vector<float, 3>(-halfWidht, -halfHeight, -halfDepth)
    );
    // Right upper front.
    result.push_back(
        entityPosition + centerOffset + Vector<float, 3>(halfWidht, halfHeight, halfDepth)
    );
    return result;
}

void setMeshBounds(MeshAxisLimitingValues meshAxisLimitingValues) {
    allMeshMaxAbsoluteValues.push_back({});

    allMeshMaxAbsoluteValues[allMeshMaxAbsoluteValues.size() - 1].absolute_x =
        (meshAxisLimitingValues.highest_x - meshAxisLimitingValues.lowest_x)
        / 2.0f;
    allMeshMaxAbsoluteValues[allMeshMaxAbsoluteValues.size() - 1].absolute_y =
        (meshAxisLimitingValues.highest_y - meshAxisLimitingValues.lowest_y)
        / 2.0f;
    allMeshMaxAbsoluteValues[allMeshMaxAbsoluteValues.size() - 1].absolute_z =
        (meshAxisLimitingValues.highest_z - meshAxisLimitingValues.lowest_z)
        / 2.0f;

    allMeshMaxAbsoluteValues[allMeshMaxAbsoluteValues.size() - 1]
        .origin_offset_x =
        (meshAxisLimitingValues.highest_x + meshAxisLimitingValues.lowest_x)
        / 2.0f;
    allMeshMaxAbsoluteValues[allMeshMaxAbsoluteValues.size() - 1]
        .origin_offset_y =
        (meshAxisLimitingValues.highest_y + meshAxisLimitingValues.lowest_y)
        / 2.0f;
    allMeshMaxAbsoluteValues[allMeshMaxAbsoluteValues.size() - 1]
        .origin_offset_z =
        (meshAxisLimitingValues.highest_z + meshAxisLimitingValues.lowest_z)
        / 2.0f;
}

void CreateProjectile(
    const Vector<float, 3>& projectilePosition,
    const Vector<float, 3>& projectileForward,
    const ecs::components::MeshHandle& meshHandle,
    const ecs::components::material& material,
    const ecs::components::damage& damage,
    const ecs::arch::EntityLocation& projectileLocation
) {
    ecs::arch::ProjectileArchetype* projectileArch =
        static_cast<ecs::arch::ProjectileArchetype*>(projectileLocation.arch);
    const uint32_t projectileIndex = projectileLocation.index;

    ecs::components::mesh* projectileMesh =
        &projectileArch->meshes[projectileIndex];
    projectileMesh->handle = meshHandle;

    ecs::arch::ProjectileBundle* projectileBundle =
        &projectileArch->projectileBundles[projectileIndex];
    projectileBundle->material = material;

    ecs::components::transform* rTransformProjectile =
        &projectileArch->transforms[projectileIndex];
    ecs::components::health* projectileHealth =
        &projectileArch->heath[projectileIndex];
    projectileHealth->maxHealth = 100;
    projectileHealth->currentHealth = 100;

    projectileArch->colliders[projectileIndex].colliders.clear();

    rTransformProjectile->scale = 0.1f;
    rTransformProjectile->position = projectilePosition;
    rTransformProjectile->forward = projectileForward;
    rTransformProjectile->position += rTransformProjectile->forward;

    projectileBundle->damage = damage;
}
}; // namespace glvm::core
