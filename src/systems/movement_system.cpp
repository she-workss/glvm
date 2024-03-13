// This file is part of Game Loop Versatile Modules (GLVM)
// Copyright © 2024 Maksim Manokhin a.k.a. Yuriorkis_Scream. Contacts:
// <fellfrostqtw@gmail.com> Author: Maksim Manokhin a.k.a. Yuriorkis_Scream
// License: http://opensource.org/licenses/MIT

#include "systems/movement_system.hpp"
#include "component_manager.hpp"
#include "components/collider_component.hpp"
#include "components/controller_component.hpp"
#include "components/directional_light_component.hpp"
#include "components/event_component.hpp"
#include "components/material_component.hpp"
#include "components/move_component.hpp"
#include "components/point_light_component.hpp"
#include "components/projectile_component.hpp"
#include "components/rigid_body_component.hpp"
#include "components/spot_light_component.hpp"
#include "components/transform_component.hpp"
#include "components/vertex_component.hpp"
#include "components/view_component.hpp"
#include "engine.hpp"
#include "entity_manager.hpp"
#include "event.hpp"
#include "i_sound_engine.hpp"
#include "vector.hpp"
#include "vertex_math.hpp"
#include <cstdio>

namespace GLVM::ecs {
CMovementSystem::CMovementSystem(core::CStack &inputStack)
    : inputStack(inputStack) {
}

void CMovementSystem::Update() {
    namespace cm = GLVM::ecs::components;

    ComponentManager *componentManager =
            GLVM::ecs::ComponentManager::GetInstance();
    core::vector<Entity> linkedEntities =
            componentManager->collectLinkedEntities<
                    cm::controller, cm::beholder, cm::transform>();
    unsigned int linkedEntitiesVectorSize = linkedEntities.GetSize();
    float cameraSpeed = 5.5f * deltaFrameTime;

    for (unsigned int i = 0; i < linkedEntitiesVectorSize; ++i) {
        Entity currentEntity = linkedEntities[i];
        cm::beholder *beholderComponent =
                componentManager->GetComponent<cm::beholder>(currentEntity);
        for (int n = 0; n < 6; ++n) {
            vec3 right;
            vec3 forward;
            switch (inputStack[n]) {
                case core::EEvents::eMOVE_LEFT:
                    right = CalculateVectorRL(*beholderComponent);
                    componentManager->CreateComponent<cm::move>(currentEntity);
                    componentManager->GetComponent<cm::move>(currentEntity)
                            ->frameMovement -= right * cameraSpeed;
                    break;
                case core::EEvents::eMOVE_RIGHT:
                    right = CalculateVectorRL(*beholderComponent);
                    componentManager->CreateComponent<cm::move>(currentEntity);
                    componentManager->GetComponent<cm::move>(currentEntity)
                            ->frameMovement += right * cameraSpeed;
                    break;
                case core::EEvents::eMOVE_BACKWARD:
                    forward = CalculateVectorFB(*beholderComponent, g_eEvent);
                    componentManager->CreateComponent<cm::move>(currentEntity);
                    componentManager->GetComponent<cm::move>(currentEntity)
                            ->frameMovement -= forward * cameraSpeed;
                    break;
                case core::EEvents::eMOVE_FORWARD:
                    forward = CalculateVectorFB(*beholderComponent, g_eEvent);
                    componentManager->CreateComponent<cm::move>(currentEntity);
                    componentManager->GetComponent<cm::move>(currentEntity)
                            ->frameMovement += forward * cameraSpeed;
                    break;
                case core::EEvents::eJUMP: {
                    cm::collider *collider =
                            componentManager->GetComponent<cm::collider>(
                                    currentEntity);
                    if (collider->bGround_Collision_) {
                        cm::rigidBody *rigidBody =
                                componentManager->GetComponent<cm::rigidBody>(
                                        currentEntity);
                        rigidBody->jumpAccumulator = 1.5f;
                    }
                } break;
                default:
                    break;
            }
            // FIXME: Flashlight crutch
        }
    }
    // FIXME: No need to have special field for gravity frame movement
    for (unsigned int n = 0;
         n < componentManager->GetEntityContainer<cm::rigidBody>()->GetSize();
         ++n) {
        int iEntity_refRigidBody =
                (*componentManager->GetEntityContainer<cm::rigidBody>())[n];
        cm::transform *rTransform_Component =
                componentManager->GetComponent<cm::transform>(
                        iEntity_refRigidBody);
        cm::rigidBody *rigidBodyComponennt =
                componentManager->GetComponent<cm::rigidBody>(
                        iEntity_refRigidBody);
        componentManager->CreateComponent<cm::move>(iEntity_refRigidBody);
        cm::move *moveComponent =
                componentManager->GetComponent<cm::move>(iEntity_refRigidBody);
        rTransform_Component->GravityAccumulator += deltaFrameTime;
        float gravity = 9.8f * rTransform_Component->GravityAccumulator *
                        rigidBodyComponennt->fMass_ * 0.0005;
        if (gravity > 0.2f) {
            gravity = 0.2;
        }

        moveComponent->gravity[1] -= gravity;
    }
}

Vector<float, 3>
CMovementSystem::CalculateVectorRL(components::beholder &beholder) {
    Vector<float, 3> normalizedVector =
            Normalize(Cross(beholder.forward, beholder.up));
    return normalizedVector;
}

Vector<float, 3>
CMovementSystem::CalculateVectorFB(components::beholder &beholder,
                                   core::CEvent &event) {
    Vector<float, 3> forward(0.0f);
    float sinYaw = std::sin(Radians(-event.mousePointerPosition.yaw / 2));
    float cosYaw = std::cos(Radians(-event.mousePointerPosition.yaw / 2));

    Quaternion yawQuat;
    yawQuat.w = cosYaw;
    yawQuat.x = 0.0f;
    yawQuat.y = sinYaw;
    yawQuat.z = 0.0f;

    Quaternion result;
    result = multiplyQuaternion(
            multiplyQuaternion(
                    yawQuat,
                    Quaternion {.w = 0.0f, .x = 0.0f, .y = 0.0f, .z = -1.0f}),
            inverseQuaternion(yawQuat));

    forward[0] = result.x;
    forward[1] = result.y;
    forward[2] = result.z;

    beholder.forward = Normalize(forward);
    return beholder.forward;
}
} // namespace GLVM::ecs
