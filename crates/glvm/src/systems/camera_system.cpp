// This file is part of Game Loop Versatile Modules (GLVM)
// Copyright © 2024 Maksim Manokhin a.k.a. Yuriorkis_Scream. Contacts:
// <fellfrostqtw@gmail.com> Author: Maksim Manokhin a.k.a. Yuriorkis_Scream
// License: http://opensource.org/licenses/MIT

#include "glvm/systems/camera_system.hpp"

#include "glvm/components/transform_component.hpp"
#include "glvm/components/view_component.hpp"
#include "glvm/vertex_math.hpp"

namespace GLVM::ecs {
void CCameraSystem::Update() {
    namespace cm = GLVM::ecs::components;

    ComponentManager* componentManager =
        GLVM::ecs::ComponentManager::GetInstance();
    core::vector<Entity> linkedEntities =
        componentManager->collectLinkedEntities<cm::beholder>();
    unsigned int linkedEntitiesVectorSize = linkedEntities.GetSize();
    for (unsigned int i = 0; i < linkedEntitiesVectorSize; ++i) {
        Entity currentEntity = linkedEntities[i];
        cm::beholder* beholderComponent =
            componentManager->GetComponent<cm::beholder>(currentEntity);
        cm::transform* transformComponent =
            componentManager->GetComponent<cm::transform>(currentEntity);
        SetViewMatrix(*transformComponent, *beholderComponent);
    }
}

void CCameraSystem::SetViewMatrix(
    components::transform& _Player,
    components::beholder& cameraComponent
) {
    Matrix<float, 4> tView_Matrix(1.0f);
    const float kSensitivity = 0.1f;

    fYaw = g_eEvent.mousePointerPosition.offset_X;
    fPitch = g_eEvent.mousePointerPosition.offset_Y;
    fYaw *= kSensitivity;
    fPitch *= kSensitivity;

    g_eEvent.mousePointerPosition.pitch = fPitch;
    g_eEvent.mousePointerPosition.yaw = fYaw;

    if (fPitch > 89.0f) {
        fPitch = 89.0f;
    }
    if (fPitch < -89.0f) {
        fPitch = -89.0f;
    }

    Vector<float, 3> front;
    front[0] = std::cos(Radians(fYaw)) * std::cos(Radians(fPitch));
    front[1] = std::sin(Radians(fPitch));
    front[2] = std::sin(Radians(fYaw)) * std::cos(Radians(fPitch));
    cameraComponent.forward = Normalize(front);

    tView_Matrix = LookAtMain(
        _Player.tPosition,
        _Player.tPosition + cameraComponent.forward,
        cameraComponent.up
    );
    SetProjectionMatrix();
}

void CCameraSystem::SetProjectionMatrix() {
    tProjection_Matrix =
        Perspective(Radians(90.0f), (float)2880 / (float)1800, 1.0f, 100.0f);
}
} // namespace GLVM::ecs
