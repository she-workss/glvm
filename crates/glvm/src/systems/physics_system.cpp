#include "glvm/systems/physics_system.hpp"

#include "glvm/archetype_ecs/arch_ecs_types.hpp"
#include "glvm/archetype_ecs/arch_ecs_world.hpp"
#include "glvm/archetypes/enemy_archetype.hpp"
#include "glvm/archetypes/item_archetype.hpp"
#include "glvm/archetypes/player_archetype.hpp"
#include "glvm/archetypes/projectile_archetype.hpp"
#include "glvm/archetypes/static_mesh_archetype.hpp"
#include "glvm/component_manager.hpp"
#include "glvm/components/attack_component.hpp"
#include "glvm/components/collider_component.hpp"
#include "glvm/components/collider_flags_component.hpp"
#include "glvm/components/font_component.hpp"
#include "glvm/components/move_component.hpp"
#include "glvm/components/rigid_body_component.hpp"
#include "glvm/components/transform_component.hpp"
#include "glvm/components/view_component.hpp"
#include "glvm/entity_manager.hpp"
#include "glvm/event.hpp"
#include "glvm/globals.hpp"
#include "glvm/systems/damage_system.hpp"
#include "glvm/vertex_math.hpp"

namespace glvm::ecs {
namespace {
bool aabbOverlap(
    const Vector<float, 3>& aPosition,
    const core::MeshAxisMaxAbsoluteValues& aBounds,
    float aScale,
    const Vector<float, 3>& bPosition,
    const core::MeshAxisMaxAbsoluteValues& bBounds,
    float bScale
) {
    return aPosition[0] + aBounds.origin_offset_x * aScale
            + aBounds.absolute_x * aScale
        > bPosition[0] + bBounds.origin_offset_x * bScale
            - bBounds.absolute_x * bScale
        && aPosition[0] + aBounds.origin_offset_x * aScale
            - aBounds.absolute_x * aScale
        < bPosition[0] + bBounds.origin_offset_x * bScale
            + bBounds.absolute_x * bScale
        && aPosition[1] + aBounds.origin_offset_y * aScale
            + aBounds.absolute_y * aScale
        > bPosition[1] + bBounds.origin_offset_y * bScale
            - bBounds.absolute_y * bScale
        && aPosition[1] + aBounds.origin_offset_y * aScale
            - aBounds.absolute_y * aScale
        < bPosition[1] + bBounds.origin_offset_y * bScale
            + bBounds.absolute_y * bScale
        && aPosition[2] + aBounds.origin_offset_z * aScale
            + aBounds.absolute_z * aScale
        > bPosition[2] + bBounds.origin_offset_z * bScale
            - bBounds.absolute_z * bScale
        && aPosition[2] + aBounds.origin_offset_z * aScale
            - aBounds.absolute_z * aScale
        < bPosition[2] + bBounds.origin_offset_z * bScale
            + bBounds.absolute_z * bScale;
}

bool isAbove(
    const Vector<float, 3>& aPosition,
    const core::MeshAxisMaxAbsoluteValues& aBounds,
    float aScale,
    const Vector<float, 3>& bPosition,
    const core::MeshAxisMaxAbsoluteValues& bBounds,
    float bScale
) {
    constexpr float epsilon = 0.15f;
    return aPosition[1] + aBounds.origin_offset_y * aScale
        - aBounds.absolute_y * aScale + epsilon
        > bPosition[1] + bBounds.origin_offset_y * bScale
        + bBounds.absolute_y * bScale;
}
} // namespace

// This update searching for referring to colliders entities and check their
// transform components for collision, and if collision detected check if
// backtracking entity had gravity component for call Gravity function.
void CPhysicsSystem::Update() {
    namespace cm = glvm::ecs::components;

    cachedArchetypesNumber = 0;
    arch::world.searchCacheArchetypes(
        requiredMask,
        archView.cachedArchetypes,
        cachedArchetypesNumber
    );

    for (uint32_t x = 0; x < cachedArchetypesNumber; ++x) {
        arch::Archetype* arch = archView.cachedArchetypes[x];

        componentsView.transformsView =
            (ecs::components::transform*)archView.cachedArchetypes[x]
                ->components[arch::ComponentsIndices::TRANSFORM_COMPONENT];
        componentsView.movesView =
            (ecs::components::move*)archView.cachedArchetypes[x]
                ->components[arch::ComponentsIndices::MOVE_COMPONENT];
        componentsView.rigidBodiesView =
            (ecs::components::rigidBody*)archView.cachedArchetypes[x]
                ->components[arch::ComponentsIndices::RIGID_BODY_COMPONENT];
        componentsView.colliderFlagsView =
            (ecs::components::colliderFlags*)archView.cachedArchetypes[x]
                ->components[arch::ComponentsIndices::COLLIDER_FLAGS_COMPONENT];
        componentsView.collidersView =
            (ecs::components::collider*)archView.cachedArchetypes[x]
                ->components[arch::ComponentsIndices::COLLIDER_COMPONENT];
        componentsView.meshesView =
            (ecs::components::mesh*)archView.cachedArchetypes[x]
                ->components[arch::ComponentsIndices::MESH_COMPONENT];

        float deltaTime = 5.5f * fDelta_Time_;
        for (unsigned int i = 0; i < arch->entityCount; ++i) {
            if (componentsView.transformsView
                && componentsView.colliderFlagsView && componentsView.movesView
                && componentsView.rigidBodiesView) {
                cm::transform& transformComponent =
                    componentsView.transformsView[i];
                cm::move& move = componentsView.movesView[i];
                cm::colliderFlags& colliderFlags =
                    componentsView.colliderFlagsView[i];
                uint8_t isGroudCollisionMask =
                    (0u << 0) | (1u << 1) | (0u << 2) | (0u << 3);
                if (colliderFlags.flags & isGroudCollisionMask) {
                    move.gravity = 0;
                    transformComponent.gravityAccumulator = 0.0f;
                }
                uint8_t isWallCollisionMask =
                    (1u << 0) | (0u << 1) | (0u << 2) | (0u << 3);
                if (colliderFlags.flags & isWallCollisionMask) {
                    // Wall-slide: zero only the frameMovement axis blocked by
                    // a collider, keep the tangential component so the player
                    // slides along the wall instead of sticking to it.
                    cm::collider* colliders = componentsView.collidersView;
                    cm::mesh* meshes = componentsView.meshesView;
                    if (colliders && meshes
                        && colliders[i].colliders.size() > 0) {
                        const core::MeshAxisMaxAbsoluteValues playerBounds =
                            allMeshMaxAbsoluteValues[meshes[i].handle.id];
                        const Vector<float, 3> playerPosition = transformComponent.position;
                        for (uint32_t c = 0; c < colliders[i].colliders.size();
                             ++c) {
                            const uint32_t collidedEntity =
                                colliders[i].colliders[c];
                            arch::EntityLocation& collidedLocation =
                                arch::world
                                    .entityLocations[arch::getId(collidedEntity)];
                            arch::Archetype* collidedArch =
                                collidedLocation.arch;
                            if (collidedArch == nullptr) {
                                // Entity was removed this frame (e.g. by
                                // DamageSystem) after collision detection.
                                continue;
                            }
                            const uint32_t collidedIndex =
                                collidedLocation.index;
                            cm::transform* collidedTransform =
                                (cm::transform*)collidedArch
                                    ->components[arch::ComponentsIndices::
                                                     TRANSFORM_COMPONENT];
                            cm::mesh* collidedMesh =
                                (cm::mesh*)collidedArch->components
                                    [arch::ComponentsIndices::MESH_COMPONENT];
                            if (!collidedTransform || !collidedMesh) {
                                continue;
                            }
                            collidedTransform += collidedIndex;
                            collidedMesh += collidedIndex;
                            const core::MeshAxisMaxAbsoluteValues collidedBounds =
                                allMeshMaxAbsoluteValues[collidedMesh->handle.id];
                            const Vector<float, 3> collidedPosition =
                                collidedTransform->position;
                            // Ground (player standing above) is handled by
                            // gravity, only resolve wall-like colliders.
                            if (isAbove(
                                    playerPosition,
                                    playerBounds,
                                    transformComponent.scale,
                                    collidedPosition,
                                    collidedBounds,
                                    collidedTransform->scale
                                )) {
                                continue;
                            }
                            for (int axis = 0; axis < 3; ++axis) {
                                Vector<float, 3> candidate = playerPosition;
                                candidate[axis] += move.frameMovement[axis];
                                bool hit = aabbOverlap(
                                    candidate,
                                    playerBounds,
                                    transformComponent.scale,
                                    collidedPosition,
                                    collidedBounds,
                                    collidedTransform->scale
                                );
                                if (hit) {
                                    move.frameMovement[axis] = 0.0f;
                                }
                            }
                        }
                    } else {
                        move.frameMovement = 0;
                    }
                    uint8_t wallCollisionTurnOffMask =
                        (0u << 0) | (1u << 1) | (1u << 2) | (1u << 3);
                    colliderFlags.flags &= wallCollisionTurnOffMask;
                }
                transformComponent.position += move.frameMovement;
                transformComponent.position += move.gravity;
                move.gravity = 0.0f;
                move.frameMovement = 0.0f;
                cm::rigidBody& rigidBody = componentsView.rigidBodiesView[i];
                if (rigidBody.jumpAccumulator > 0.0f) {
                    rigidBody.jumpAccumulator -= deltaTime;
                    Vector<float, 3> jump = Vector<float, 3> {0.0f, 5.0f, 0.0f} * deltaTime;
                    transformComponent.position += jump;
                }
            }
        }
    }
}
} // namespace glvm::ecs
