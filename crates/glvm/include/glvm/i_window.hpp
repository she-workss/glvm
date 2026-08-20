#pragma once

#include "glvm/Event.hpp"

namespace glvm::core {

class IWindow {
public:
    // Window keyboard focus, updated by each backend.
    bool isFocused = true;

    virtual ~IWindow() = default;

    virtual void SwapBuffers() = 0;
    virtual void ClearDisplay() = 0;
    virtual bool HandleEvent(CEvent& _Event) = 0;
    virtual void Close() = 0;
    virtual void CursorLock(
        int _x_position,
        int _y_position,
        int* _x_offset,
        int* _y_offset
    ) = 0;
};

} // namespace glvm::core
