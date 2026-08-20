#pragma once

#include "glvm/events_stack.hpp"
#include "glvm/globals.hpp"
#include "glvm/i_window.hpp"

#include <X11/Xlib.h>

#define XKEY_I 0x69
#define XKEY_ESCAPE 0xff1b
#define XKEY_A 0x61
#define XKEY_D 0x64
#define XKEY_S 0x73
#define XKEY_W 0x77
#define XKEY_SPACE 0x20

namespace glvm::core {
class WindowXVulkan: public IWindow {
    XWindowAttributes GWindow_Attributes_;
    Window Root_Window_;
    XSetWindowAttributes Set_Window_Attributes_;

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
} // namespace glvm::core
