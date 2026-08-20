#include "glvm/Event.hpp"

#include "glvm/EventsStack.hpp"

namespace glvm::core {
CEvent::CEvent() {}

EEvents& CEvent::GetEvent() {
    return eEvent_;
}

void CEvent::SetEvent(EEvents _eEvent) {
    eEvent_ = _eEvent;
}

void CEvent::SetNextEvent(EEvents _eEvent) {
    nextEvent = _eEvent;
}

EEvents CEvent::GetNextEvent() {
    return nextEvent;
}

void CEvent::SetLastEvent(CStack _Stack) {
    switch (_Stack.Pop()) {
        case glvm::core::eMOVE_RIGHT:
            SetEvent(glvm::core::EEvents::eMOVE_RIGHT);
            break;
        case glvm::core::eMOVE_LEFT:
            SetEvent(glvm::core::EEvents::eMOVE_LEFT);
            break;
        case glvm::core::eMOVE_BACKWARD:
            SetEvent(glvm::core::EEvents::eMOVE_BACKWARD);
            break;
        case glvm::core::eMOVE_FORWARD:
            SetEvent(glvm::core::EEvents::eMOVE_FORWARD);
            break;
        case glvm::core::eMOUSE_LEFT_BUTTON:
            SetEvent(glvm::core::EEvents::eMOUSE_LEFT_BUTTON);
            break;
        default:
            break;
    }
}
} // namespace glvm::core
