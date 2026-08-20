#pragma once

#include "glvm/Event.hpp"
#include "glvm/Stack.hpp"

namespace glvm::core {
class ControllerSystem {
    CStack& inputStack_;
    CEvent event_;

public:
    ControllerSystem(CStack& inputStack, CEvent& event);
    void Update();
};
} // namespace glvm::core
