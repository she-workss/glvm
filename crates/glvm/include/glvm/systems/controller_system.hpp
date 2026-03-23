// This file is part of Game Loop Versatile Modules (GLVM)
// Copyright © 2024 Maksim Manokhin a.k.a. Yuriorkis_Scream. Contacts:
// <fellfrostqtw@gmail.com> Author: Maksim Manokhin a.k.a. Yuriorkis_Scream
// License: http://opensource.org/licenses/MIT
#pragma once

#include "glvm/event.hpp"
#include "glvm/stack.hpp"

namespace GLVM::core {
class ControllerSystem {
    CStack& inputStack_;
    CEvent event_;

public:
    ControllerSystem(CStack& inputStack, CEvent& event);
    void Update();
};
} // namespace GLVM::core
