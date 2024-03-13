// This file is part of Game Loop Versatile Modules (GLVM)
// Copyright © 2024 Maksim Manokhin a.k.a. Yuriorkis_Scream. Contacts:
// <fellfrostqtw@gmail.com> Author: Maksim Manokhin a.k.a. Yuriorkis_Scream
// License: http://opensource.org/licenses/MIT

#include "systems/collision_system.hpp"
#include "component_manager.hpp"
#include "components/collider_component.hpp"
#include "components/event_component.hpp"
#include "components/material_component.hpp"
#include "components/move_component.hpp"
#include "components/rigid_body_component.hpp"
#include "components/transform_component.hpp"
#include "components/view_component.hpp"
#include "entity_manager.hpp"
#include "event.hpp"
#include "events_stack.hpp"
#include "vector.hpp"
#include "vertex_math.hpp"

namespace GLVM::ecs {
bool CCollisionSystem::BoxCollider(vec3 backtrackingPosition,
                                   vec3 comparedPosition,
                                   float backtrackingScale,
                                   float comparedScale) {
    if (backtrackingPosition[0] + backtrackingScale >
                comparedPosition[0] - comparedScale &&
        backtrackingPosition[0] - backtrackingScale <
                comparedPosition[0] + comparedScale &&
        backtrackingPosition[1] + backtrackingScale >
                comparedPosition[1] - comparedScale &&
        backtrackingPosition[1] - backtrackingScale <
                comparedPosition[1] + comparedScale &&
        backtrackingPosition[2] + backtrackingScale >
                comparedPosition[2] - comparedScale &&
        backtrackingPosition[2] - backtrackingScale <
                comparedPosition[2] + comparedScale) {
        return true;
    }

    return false;
}

bool CCollisionSystem::UpperActorCheck(vec3 backtrackingPosition,
                                       vec3 comparedPosition,
                                       float backtrackingScale,
                                       float comparedScale) {
    if ((backtrackingPosition[1] - backtrackingScale) + 0.01f >
        (comparedPosition[1] + (comparedScale))) {
        return true;
    }

    return false;
}

void CCollisionSystem::Update() {
    namespace cm = GLVM::ecs::components;

    ComponentManager *componentManager = ComponentManager::GetInstance();
    core::vector<Entity> linkedEntities =
            componentManager
                    ->collectLinkedEntities<cm::collider, cm::transform>();

    core::vector<Entity> linkedEntitiesWithMove =
            componentManager->collectLinkedEntities<cm::collider, cm::transform,
                                                    cm::move>();

    float cameraSpeed = 5.5f * fDelta_Time_;
    unsigned int linkedEntitiesVectorSize = linkedEntities.GetSize();
    unsigned int linkedEntitiesVectorSizeWithMove =
            linkedEntitiesWithMove.GetSize();
    for (unsigned int i = 0; i < linkedEntitiesVectorSize; ++i) {
        unsigned int backtrackingEntityRefCollider = linkedEntities[i];
        componentManager
                ->GetComponent<cm::collider>(backtrackingEntityRefCollider)
                ->bGround_Collision_ = false;
        for (unsigned int j = 0; j < linkedEntitiesVectorSize; ++j) {
            if (i == j) {
                continue;
            }

            unsigned int comparedEntityRefCollider = linkedEntities[j];

            vec3 backtrackingTransform =
                    componentManager
                            ->GetComponent<cm::transform>(
                                    backtrackingEntityRefCollider)
                            ->tPosition;
            vec3 backtrackingTransformUpper =
                    componentManager
                            ->GetComponent<cm::transform>(
                                    backtrackingEntityRefCollider)
                            ->tPosition;
            float backtrackingScale =
                    componentManager
                            ->GetComponent<cm::transform>(
                                    backtrackingEntityRefCollider)
                            ->fScale;
            float backtrackingGltfFlag =
                    componentManager
                            ->GetComponent<cm::transform>(
                                    backtrackingEntityRefCollider)
                            ->gltf;
            vec3 comparedTransform = componentManager
                                             ->GetComponent<cm::transform>(
                                                     comparedEntityRefCollider)
                                             ->tPosition;
            vec3 comparedTransformUpper =
                    componentManager
                            ->GetComponent<cm::transform>(
                                    comparedEntityRefCollider)
                            ->tPosition;
            float comparedScale = componentManager
                                          ->GetComponent<cm::transform>(
                                                  comparedEntityRefCollider)
                                          ->fScale;
            float comparedGltfFlag = componentManager
                                             ->GetComponent<cm::transform>(
                                                     comparedEntityRefCollider)
                                             ->gltf;
            for (unsigned int m = 0; m < linkedEntitiesVectorSizeWithMove;
                 ++m) {
                if (backtrackingEntityRefCollider ==
                    linkedEntitiesWithMove[m]) {
                    cm::move *backtrackingMove =
                            componentManager->GetComponent<cm::move>(
                                    backtrackingEntityRefCollider);
                    backtrackingTransform +=
                            Normalize(backtrackingMove->frameMovement) *
                            cameraSpeed;
                    backtrackingTransform += backtrackingMove->gravity;
                }
            }
            for (unsigned int n = 0; n < linkedEntitiesVectorSizeWithMove;
                 ++n) {
                if (comparedEntityRefCollider == linkedEntitiesWithMove[n]) {
                    cm::move *comparedMove =
                            componentManager->GetComponent<cm::move>(
                                    comparedEntityRefCollider);
                    comparedTransform +=
                            Normalize(comparedMove->frameMovement) *
                            cameraSpeed;
                    comparedTransform += comparedMove->gravity;
                }
            }

            if (!backtrackingGltfFlag) {
                backtrackingScale /= 2;
            }

            if (!comparedGltfFlag) {
                comparedScale /= 2;
            }

            bool boxColliderFlag;
            bool upperActorCheckFlag = false;
            boxColliderFlag =
                    BoxCollider(backtrackingTransform, comparedTransform,
                                backtrackingScale, comparedScale);
            if (boxColliderFlag) {
                upperActorCheckFlag = UpperActorCheck(
                        backtrackingTransformUpper, comparedTransformUpper,
                        backtrackingScale, comparedScale);
            }

            if (upperActorCheckFlag && boxColliderFlag) {
                componentManager
                        ->GetComponent<cm::collider>(
                                backtrackingEntityRefCollider)
                        ->bGround_Collision_ = true;
                continue;
            }

            if (boxColliderFlag) {
                componentManager
                        ->GetComponent<cm::collider>(
                                backtrackingEntityRefCollider)
                        ->bWall_Collision_ = true;
                continue;
            }
        }
    }
}

} // namespace GLVM::ecs
