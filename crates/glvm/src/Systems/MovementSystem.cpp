// This file is part of Game Loop Versatile Modules (GLVM)
// Copyright © 2024 Maksim Manokhin a.k.a. Yuriorkis_Scream. Contacts:
// <fellfrostqtw@gmail.com> Author: Maksim Manokhin a.k.a. Yuriorkis_Scream
// License: http://opensource.org/licenses/MIT

#include "glvm/Systems/MovementSystem.hpp"

#include "glvm/ArchetypeECS/ArchECS_Types.hpp"
#include "glvm/ArchetypeECS/ArchECS_World.hpp"
#include "glvm/ArchetypeECS/ArchetypeInterface.hpp"
#include "glvm/Archetypes/EnemyArchetype.hpp"
#include "glvm/Archetypes/ItemArchetype.hpp"
#include "glvm/Archetypes/PlayerArchetype.hpp"
#include "glvm/Components/ColliderFlagsComponent.hpp"
#include "glvm/Components/MoveComponent.hpp"
#include "glvm/Components/RigidBodyComponent.hpp"
#include "glvm/Components/TransformComponent.hpp"
#include "glvm/Components/ViewComponent.hpp"
#include "glvm/Event.hpp"
#include "glvm/VertexMath.hpp"

#include <cstdint>
#include <sys/types.h>

namespace GLVM::ecs {
CMovementSystem::CMovementSystem(core::CStack& inputStack) :
    inputStack(inputStack) {}

void CMovementSystem::Update() {
    namespace cm = GLVM::ecs::components;
    namespace arch = GLVM::ecs::arch;

    arch::world.searchCacheArchetypes(
        playerRequiredMask,
        &archView.playerCachedArchetype,
        playerArchetypesNumber
    );
    componentsView.playerMoves =
        (ecs::components::move*)archView.playerCachedArchetype
            ->components[arch::ComponentsIndices::MOVE_COMPONENT];
    componentsView.playerViews =
        (ecs::components::beholder*)archView.playerCachedArchetype
            ->components[arch::ComponentsIndices::VIEW_COMPONENT];
    componentsView.playerColliderFlags =
        (ecs::components::colliderFlags*)archView.playerCachedArchetype
            ->components[arch::ComponentsIndices::COLLIDER_FLAGS_COMPONENT];
    componentsView.playerRigidBody =
        (ecs::components::rigidBody*)archView.playerCachedArchetype
            ->components[arch::ComponentsIndices::RIGID_BODY_COMPONENT];

    const float cameraSpeed = 3.0f * deltaFrameTime;
    for (unsigned int i = 0; i < archView.playerCachedArchetype->entityCount;
         ++i) {
        const arch::entity entity = archView.playerCachedArchetype->entities[i];
        ecs::arch::EntityLocation& entityLocation =
            ecs::arch::world.entityLocations[ecs::arch::getId(entity)];
        cm::beholder* playerView = &componentsView.playerViews[i];
        cm::move* playerMove = &componentsView.playerMoves[i];
        cm::colliderFlags* playerColliderFlags =
            &componentsView.playerColliderFlags[i];
        cm::rigidBody* playerRigidBody = &componentsView.playerRigidBody[i];
        for (int n = 0; n < 6; ++n) {
            vec3 right;
            vec3 forward;
            switch (inputStack[n]) {
                case core::EEvents::eMOVE_LEFT:
                    right = CalculateVectorRL(*playerView);
                    playerMove->frameMovement -= right * cameraSpeed;
                    entityLocation.isDirty = true;
                    break;
                case core::EEvents::eMOVE_RIGHT:
                    right = CalculateVectorRL(*playerView);
                    playerMove->frameMovement += right * cameraSpeed;
                    entityLocation.isDirty = true;
                    break;
                case core::EEvents::eMOVE_BACKWARD:
                    forward = CalculateVectorFB(*playerView, g_eEvent);
                    playerMove->frameMovement -= forward * cameraSpeed;
                    entityLocation.isDirty = true;
                    break;
                case core::EEvents::eMOVE_FORWARD:
                    forward = CalculateVectorFB(*playerView, g_eEvent);
                    playerMove->frameMovement += forward * cameraSpeed;
                    entityLocation.isDirty = true;
                    break;
                case core::EEvents::eJUMP: {
                    entityLocation.isDirty = true;
                    uint8_t isGroudCollisionMask =
                        (0u << 0) | (1u << 1) | (0u << 2) | (0u << 3);
                    if (playerColliderFlags->flags & isGroudCollisionMask) {
                        playerRigidBody->jumpAccumulator = 1.5f;
                    }
                } break;
                default:
                    break;
            }
        }
    }

    rigidBodyContainedArchetypesNumber = 0;
    arch::world.searchCacheArchetypes(
        rigidBodyRequiredMask,
        archView.rigidBodyContainedArchetypesCache,
        rigidBodyContainedArchetypesNumber
    );

    for (uint32_t i0 = 0; i0 < rigidBodyContainedArchetypesNumber; ++i0) {
        arch::Archetype* currentArch =
            archView.rigidBodyContainedArchetypesCache[i0];
        componentsView.transforms =
            (ecs::components::transform*)currentArch
                ->components[arch::ComponentsIndices::TRANSFORM_COMPONENT];
        componentsView.rigidBodies =
            (ecs::components::rigidBody*)currentArch
                ->components[arch::ComponentsIndices::RIGID_BODY_COMPONENT];
        componentsView.moves =
            (ecs::components::move*)currentArch
                ->components[arch::ComponentsIndices::MOVE_COMPONENT];
        componentsView.items =
            (ecs::components::item*)currentArch
                ->components[arch::ComponentsIndices::ITEM_COMPONENT];

        for (uint32_t i1 = 0; i1 < currentArch->entityCount; ++i1) {
            const arch::entity entity = currentArch->entities[i1];
            ecs::arch::EntityLocation& entityLocation =
                ecs::arch::world.entityLocations[ecs::arch::getId(entity)];
            entityLocation.isDirty = true;

            if (componentsView.items && !componentsView.items[i1].isActor) {
                continue;
            }

            cm::transform* rTransform_Component =
                &componentsView.transforms[i1];
            cm::rigidBody* rigidBodyComponennt =
                &componentsView.rigidBodies[i1];
            cm::move* moveComponent = &componentsView.moves[i1];
            rTransform_Component->gravityAccumulator += deltaFrameTime;
            float gravity = 9.8f * rTransform_Component->gravityAccumulator
                * rigidBodyComponennt->fMass_ * 0.0005;
            if (gravity > 0.2f) {
                gravity = 0.2;
            }

            moveComponent->gravity[1] -= gravity;
        }
    }
}

Vector<float, 3> CMovementSystem::CalculateVectorRL(
    components::beholder& beholder
) {
    Vector<float, 3> normalizedVector =
        Normalize(Cross(beholder.forward, vec3 {0.0f, -1.0f, 0.0}));
    return normalizedVector;
}

Vector<float, 3> CMovementSystem::CalculateVectorFB(
    components::beholder& beholder,
    [[maybe_unused]] core::CEvent& event
) {
    Vector<float, 3> forward(0.0f);
    // forward[0] = std::cos(Radians(event.mousePointerPosition.yaw * 2));
    // forward[2] = std::sin(Radians(event.mousePointerPosition.yaw * 2));

    // float sinYaw = std::sin(Radians(event.mousePointerPosition.yaw / 2));
    // float cosYaw = std::cos(Radians(event.mousePointerPosition.yaw / 2));

    // Quaternion yawQuat;
    // yawQuat.w = cosYaw;
    // yawQuat.x = 0.0f;
    // yawQuat.y = sinYaw;
    // yawQuat.z = 0.0f;

    // Quaternion result;
    // result = multiplyQuaternion(multiplyQuaternion(yawQuat, Quaternion{ .w =
    // 0.0f, .x = 0.0f, 			.y = 0.0f, .z = 1.0f }),
    // inverseQuaternion(yawQuat));

    // forward[0] = result.x;
    // forward[1] = result.y;
    // forward[2] = result.z;

    current_X = (float)g_eEvent.mousePointerPosition.offset_X;
    float delta_x = current_X - prev_X;
    // if ( delta_x < 0.0001 )
    // 	delta_x = prev_delta_x;

    const vec3 rotateAxis = {0.0, -1.0, 0.0};
    float rotationAngle = delta_x;
    constexpr float angleScale = 0.1f;
    rotationAngle = Radians(rotationAngle * angleScale);
    constexpr float quatAngleCorrection =
        0.5f; /// Quaternions need devision by 2
    const float sinRotationAngle = sinf(rotationAngle * quatAngleCorrection);
    Quaternion rotationQuat = Quaternion(
        cosf(rotationAngle * quatAngleCorrection),
        sinRotationAngle * rotateAxis[0],
        sinRotationAngle * rotateAxis[1],
        sinRotationAngle * rotateAxis[2]
    );
    // Quaternion appliedRotationQuat =
    // multiplyQuaternion(multiplyQuaternion(rotationQuat, Quaternion(0.0f,
    // beholder.forward[0],
    // beholder.forward[1], beholder.forward[2])),
    // conjugate(rotationQuat));
    const Quaternion appliedRotationQuat = (rotationQuat
                                            * Quaternion(
                                                0.0f,
                                                beholder.forward[0],
                                                beholder.forward[1],
                                                beholder.forward[2]
                                            ))
        * conjugate(rotationQuat);

    forward[0] = appliedRotationQuat.x;
    forward[1] = 0.0f;
    forward[2] = appliedRotationQuat.z;

    prev_X = (float)g_eEvent.mousePointerPosition.offset_X;
    // if ( delta_x > 0.0f )
    // 	prev_delta_x = delta_x;

    forward = Normalize(forward);
    return forward;
}
} // namespace GLVM::ecs
