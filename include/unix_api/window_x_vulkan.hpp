// This file is part of Game Loop Versatile Modules (GLVM)
// Copyright © 2024 Maksim Manokhin a.k.a. Yuriorkis_Scream. Contacts:
// <fellfrostqtw@gmail.com> Author: Maksim Manokhin a.k.a. Yuriorkis_Scream
// License: http://opensource.org/licenses/MIT
#pragma once

#include "events_stack.hpp"
#include "i_window.hpp"

#include <X11/Xlib.h>

namespace GLVM::core {
class WindowXVulkan: public IWindow {
    XWindowAttributes GWindow_Attributes_;
    Window Root_Window_;
    XSetWindowAttributes Set_Window_Attributes_;

public:
    Display* pDisp_;
    Window Win_;
    CStack* Input_Stack_;

    WindowXVulkan();
    ~WindowXVulkan();

    Window GetWindow();
    Display* GetDisplay();
    void CursorLock(
        int _x_position,
        int _y_position,
        int* _x_offset,
        int* _y_offset
    ) override;
    void SwapBuffers() override;
    void ClearDisplay() override;
    bool HandleEvent(CEvent& _Event) override;
    void Close() override;
};
} // namespace GLVM::core
