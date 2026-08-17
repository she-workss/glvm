#ifdef __linux__
// This file is part of Game Loop Versatile Modules (GLVM)
// Copyright © 2024 Maksim Manokhin a.k.a. Yuriorkis_Scream. Contacts:
// <fellfrostqtw@gmail.com> Author: Maksim Manokhin a.k.a. Yuriorkis_Scream
// License: http://opensource.org/licenses/MIT

#ifndef WINDOW_X_VULKAN
#define WINDOW_X_VULKAN

#include "glvm/EventsStack.hpp"
#include "glvm/Globals.hpp"
#include "glvm/IWindow.hpp"

#include <X11/Xlib.h>

#define XKEY_I 0x69
#define XKEY_ESCAPE 0xff1b
#define XKEY_A 0x61
#define XKEY_D 0x64
#define XKEY_S 0x73
#define XKEY_W 0x77
#define XKEY_SPACE 0x20

namespace GLVM::core {
class WindowXVulkan: public IWindow {
    XWindowAttributes GWindow_Attributes_;
    Window Root_Window_;
    XSetWindowAttributes Set_Window_Attributes_;

    // XWindowAttributes gwa_;

public:
    Display* pDisp_;
    Window Win_;
    uint32_t width;
    uint32_t height;

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

#endif

#endif
