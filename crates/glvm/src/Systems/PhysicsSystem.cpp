// This file is part of Game Loop Versatile Modules (GLVM)
// Copyright © 2024 Maksim Manokhin a.k.a. Yuriorkis_Scream. Contacts:
// <fellfrostqtw@gmail.com> Author: Maksim Manokhin a.k.a. Yuriorkis_Scream
// License: http://opensource.org/licenses/MIT

#include "glvm/Systems/PhysicsSystem.hpp"

#include "glvm/ArchetypeECS/ArchECS_Types.hpp"
#include "glvm/ArchetypeECS/ArchECS_World.hpp"
#include "glvm/Archetypes/EnemyArchetype.hpp"
#include "glvm/Archetypes/ItemArchetype.hpp"
#include "glvm/Archetypes/PlayerArchetype.hpp"
#include "glvm/Archetypes/ProjectileArchetype.hpp"
#include "glvm/Archetypes/StaticMeshArchetype.hpp"
#include "glvm/ComponentManager.hpp"
#include "glvm/Components/AttackComponent.hpp"
#include "glvm/Components/ColliderComponent.hpp"
#include "glvm/Components/ColliderFlagsComponent.hpp"
#include "glvm/Components/FontComponent.hpp"
#include "glvm/Components/MoveComponent.hpp"
#include "glvm/Components/RigidBodyComponent.hpp"
#include "glvm/Components/TransformComponent.hpp"
#include "glvm/Components/ViewComponent.hpp"
#include "glvm/EntityManager.hpp"
#include "glvm/Event.hpp"
#include "glvm/Globals.hpp"
#include "glvm/Systems/DamageSystem.hpp"
#include "glvm/VertexMath.hpp"

namespace GLVM::ecs {
namespace {
bool aabbOverlap(
    const vec3& aPosition,
    const core::MeshAxisMaxAbsoluteValues& aBounds,
    float aScale,
    const vec3& bPosition,
    const core::MeshAxisMaxAbsoluteValues& bBounds,
    float bScale
) {
    return aPosition[0] + aBounds.origin_offset_x + aBounds.absolute_x * aScale
                > bPosition[0] + bBounds.origin_offset_x
                    - bBounds.absolute_x * bScale
        && aPosition[0] + aBounds.origin_offset_x - aBounds.absolute_x * aScale
            < bPosition[0] + bBounds.origin_offset_x
                + bBounds.absolute_x * bScale
        && aPosition[1] + aBounds.origin_offset_y + aBounds.absolute_y * aScale
                > bPosition[1] + bBounds.origin_offset_y
                    - bBounds.absolute_y * bScale
        && aPosition[1] + aBounds.origin_offset_y - aBounds.absolute_y * aScale
            < bPosition[1] + bBounds.origin_offset_y
                + bBounds.absolute_y * bScale
        && aPosition[2] + aBounds.origin_offset_z + aBounds.absolute_z * aScale
                > bPosition[2] + bBounds.origin_offset_z
                    - bBounds.absolute_z * bScale
        && aPosition[2] + aBounds.origin_offset_z - aBounds.absolute_z * aScale
            < bPosition[2] + bBounds.origin_offset_z
                + bBounds.absolute_z * bScale;
}

bool isAbove(
    const vec3& aPosition,
    const core::MeshAxisMaxAbsoluteValues& aBounds,
    float aScale,
    const vec3& bPosition,
    const core::MeshAxisMaxAbsoluteValues& bBounds,
    float bScale
) {
    constexpr float epsilon = 0.15f;
    return aPosition[1] + aBounds.origin_offset_y - aBounds.absolute_y * aScale
            + epsilon
        > bPosition[1] + bBounds.origin_offset_y
            + bBounds.absolute_y * bScale;
}
} // namespace

/*! This update searching for refering to colliders entities and check their
 *  transform components for collision, and if collision detected check if
 *  backtracking entity had gravity component for call Gravity function.
 */

void CPhysicsSystem::Update() {
    namespace cm = GLVM::ecs::components;

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
                //				cm::collider& collider = collidersView[i];
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
                    // wall-slide: zero only the frameMovement axis blocked by
                    // a collider, keep the tangential component so the player
                    // slides along the wall instead of sticking to it
                    cm::collider* colliders = componentsView.collidersView;
                    cm::mesh* meshes = componentsView.meshesView;
                    if (colliders && meshes
                        && colliders[i].colliders.GetSize() > 0) {
                        const core::MeshAxisMaxAbsoluteValues playerBounds =
                            allMeshMaxAbsoluteValues[meshes[i].handle.id];
                        const vec3 playerPosition =
                            transformComponent.position;
                        for (u32 c = 0; c < colliders[i].colliders.GetSize();
                             ++c) {
                            const u32 collidedEntity =
                                colliders[i].colliders[c];
                            arch::EntityLocation& collidedLocation =
                                arch::world
                                    .entityLocations[arch::getId(collidedEntity)];
                            arch::Archetype* collidedArch =
                                collidedLocation.arch;
                            if (collidedArch == nullptr) {
                                /// entity was removed this frame (e.g. by
                                /// DamageSystem) after collision detection
                                continue;
                            }
                            const uint32_t collidedIndex =
                                collidedLocation.index;
                            cm::transform* collidedTransform =
                                (cm::transform*)collidedArch->components
                                    [arch::ComponentsIndices::TRANSFORM_COMPONENT];
                            cm::mesh* collidedMesh =
                                (cm::mesh*)collidedArch->components
                                    [arch::ComponentsIndices::MESH_COMPONENT];
                            if (!collidedTransform || !collidedMesh) {
                                continue;
                            }
                            collidedTransform += collidedIndex;
                            collidedMesh += collidedIndex;
                            const core::MeshAxisMaxAbsoluteValues
                                collidedBounds =
                                    allMeshMaxAbsoluteValues[collidedMesh
                                                                ->handle
                                                                .id];
                            const vec3 collidedPosition =
                                collidedTransform->position;
                            // ground (player standing above) is handled by
                            // gravity, only resolve wall-like colliders
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
                                vec3 candidate = playerPosition;
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

                // u32 entity = arch->entities[i];
                // if( entity == 0 ) {
                // 	std::cout << "frame move: " << "x: " <<
                // move.frameMovement[0] << " y: " << move.frameMovement[1] <<
                // 		" z: " << move.frameMovement << std::endl;
                // }

                transformComponent.position += move.frameMovement;
                transformComponent.position += move.gravity;
                move.gravity = 0.0f;
                move.frameMovement = 0.0f;
                //				componentManager->RemoveComponent<cm::move>(entityRefMove);

                cm::rigidBody& rigidBody = componentsView.rigidBodiesView[i];
                if (rigidBody.jumpAccumulator > 0.0f) {
                    rigidBody.jumpAccumulator -= deltaTime;
                    vec3 jump = vec3 {0.0f, 5.0f, 0.0f} * deltaTime;
                    transformComponent.position += jump;
                }
            }
        }
    }
}
} // namespace GLVM::ecs
