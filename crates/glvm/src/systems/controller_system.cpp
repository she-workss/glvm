#include "glvm/systems/controller_system.hpp"

#include "glvm/event.hpp"

namespace glvm::core {
ControllerSystem::ControllerSystem(CStack& inputStack, CEvent& event) :
    inputStack_(inputStack),
    event_(event) {}

void ControllerSystem::Update() {}
} // namespace glvm::core
