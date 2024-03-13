#include "systems/controller_system.hpp"
#include "event.hpp"

namespace GLVM::core {
ControllerSystem::ControllerSystem(CStack &inputStack, CEvent &event)
    : inputStack_(inputStack), event_(event) {
}

void ControllerSystem::Update() {
}
} // namespace GLVM::core
