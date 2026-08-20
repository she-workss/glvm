#pragma once

#include "glvm/events_stack.hpp"
#include "glvm/globals.hpp"
#include "glvm/i_window.hpp"

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
class WindowXCBVulkan: public IWindow {
    xcb_connection_t* connection;
    xcb_screen_t* screen;
    uint32_t window;
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
    uint32_t GetWindow();
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
