// This file is part of Game Loop Versatile Modules (GLVM)
// Copyright © 2024 Maksim Manokhin a.k.a. Yuriorkis_Scream. Contacts:
// <fellfrostqtw@gmail.com> Author: Maksim Manokhin a.k.a. Yuriorkis_Scream
// License: http://opensource.org/licenses/MIT
#pragma once

#include "component_manager.hpp"
#include "components/move_component.hpp"
#include "components/spot_light_component.hpp"
#include "components/transform_component.hpp"
#include "components/view_component.hpp"
#include "entity_manager.hpp"
#include "event.hpp"
#include "events_stack.hpp"
#include "globals.hpp"
#include "i_sound_engine.hpp"
#include "i_system.hpp"
#include "vector.hpp"
#include "vertex_math.hpp"

namespace GLVM::ecs {
class CMovementSystem : public ISystem {
public:
    float deltaFrameTime;
    float gravity;
    core::CStack &inputStack;

    CMovementSystem(core::CStack &inputStack);

    void Update();
    Vector<float, 3> CalculateVectorRL(components::beholder &beholder);
    Vector<float, 3> CalculateVectorFB(components::beholder &beholder,
                                       core::CEvent &event);
};
} // namespace GLVM::ecs
