#pragma once

#include "glvm/EventsStack.hpp"
#include "glvm/Globals.hpp"
#include "glvm/IWindow.hpp"

#include <cassert>
#include <cstdint>
#include <cstdio>
#include <iostream>
#include <unistd.h>
#include <xcb/xcb.h>
#include <xcb/xcb_cursor.h>
#include <xcb/xcb_keysyms.h>
#include <xcb/xproto.h>

namespace glvm::core {
typedef uint32_t xcb_window_t;

class WindowXCBVulkan: public IWindow {
    xcb_connection_t* connection;
    xcb_screen_t* screen;
    xcb_window_t window;
    xcb_key_symbols_t* key_symbols;
    xcb_generic_event_t* next_generic_event = NULL;

    static void print_modifiers(uint32_t mask);

public:
    uint32_t width;
    uint32_t height;
    bool isWindowResizeRead = false;

    WindowXCBVulkan();

    void configureWindow();
    void HideCursor();
    xcb_connection_t* GetConnection();
    xcb_window_t GetWindow();
    void Disconnect();

    void SwapBuffers() override;
    void ClearDisplay() override;
    bool HandleEvent(CEvent& _Event) override;
    void Close() override;
    void CursorLock(
        int _x_position,
        int _y_position,
        int* _x_offset,
        int* _y_offset
    ) override;
};
} // namespace glvm::core
