// This file is part of Game Loop Versatile Modules (GLVM)
// Copyright © 2024 Maksim Manokhin a.k.a. Yuriorkis_Scream. Contacts:
// <fellfrostqtw@gmail.com> Author: Maksim Manokhin a.k.a. Yuriorkis_Scream
// License: http://opensource.org/licenses/MIT
#pragma once

#include "glvm/component_manager.hpp"
#include "glvm/components/move_component.hpp"
#include "glvm/components/spot_light_component.hpp"
#include "glvm/components/transform_component.hpp"
#include "glvm/components/view_component.hpp"
#include "glvm/entity_manager.hpp"
#include "glvm/event.hpp"
#include "glvm/events_stack.hpp"
#include "glvm/globals.hpp"
#include "glvm/i_sound_engine.hpp"
#include "glvm/i_system.hpp"
#include "glvm/vector.hpp"
#include "glvm/vertex_math.hpp"

namespace GLVM::ecs {
class CMovementSystem: public ISystem {
public:
    float deltaFrameTime;
    float gravity;
    core::CStack& inputStack;

    CMovementSystem(core::CStack& inputStack);

    void Update();
    Vector<float, 3> CalculateVectorRL(components::beholder& beholder);
    Vector<float, 3> CalculateVectorFB(
        components::beholder& beholder,
        core::CEvent& event
    );
};
} // namespace GLVM::ecs
