// This file is part of Game Loop Versatile Modules (GLVM)
// Copyright © 2024 Maksim Manokhin a.k.a. Yuriorkis_Scream. Contacts:
// <fellfrostqtw@gmail.com> Author: Maksim Manokhin a.k.a. Yuriorkis_Scream
// License: http://opensource.org/licenses/MIT

#ifndef COLLISION_SYSTEM
#define COLLISION_SYSTEM

#include "Components/ColliderComponent.hpp"
#include "Components/EventComponent.hpp"
#include "Components/MoveComponent.hpp"
#include "Components/RigidBodyComponent.hpp"
#include "Components/TransformComponent.hpp"
#include "Components/VertexComponent.hpp"
#include "Components/ViewComponent.hpp"
#include "Event.hpp"
#include "Globals.hpp"
#include "ISystem.hpp"
#include "Vector.hpp"
#include "VertexMath.hpp"
#include <mutex>

namespace GLVM::ecs {
class CCollisionSystem : public ISystem {
public:
    float fDelta_Time_;
    float gravity;
    core::CStack &Input_Stack_;

    CCollisionSystem(core::CStack &_input_Stack) : Input_Stack_(_input_Stack) {
    }
    void Repel(components::transform &_transform_Component,
               components::move &_move_Component, float &_fDelta_Time,
               components::beholder &_view_Component, core::CEvent &_event);
    bool Gravity(components::transform &_transform_Component,
                 components::event &_event_Component);
    bool BoxCollider(vec3 backtrackingPosition, vec3 comparedPosition,
                     float backtrackingScale, float comparedScale);
    void Update() override;
    bool UpperActorCheck(vec3 backtrackingPosition, vec3 comparedPosition,
                         float backtrackingScale, float comparedScale);
    bool RayCast(vec3 rayCasterPosition, vec3 receiverPosition,
                 float rayCasterScale, float receiverScale);
};
} // namespace GLVM::ecs

#endif
