#include "glvm/unix_api/window_xcb_vulkan.hpp"

#include "glvm/event.hpp"

#include <X11/X.h>
#include <X11/XKBlib.h>
#include <cstdint>
#include <xcb/xcb.h>
#include <xcb/xcb_cursor.h>
#include <xcb/xcb_keysyms.h>
#include <xcb/xfixes.h>
#include <xcb/xproto.h>

namespace glvm::core {
WindowXCBVulkan::WindowXCBVulkan() {
    // Open the connection to the X server.
    connection = xcb_connect(NULL, NULL);
    int error = xcb_connection_has_error(connection);
    if (error) {
        fprintf(stderr, "XCB connection error: %d\n", error);
        // Handle error or exit.
    }

    // Get the first screen.
    const xcb_setup_t* setup = xcb_get_setup(connection);
    assert(connection != NULL);

    xcb_screen_iterator_t iterator = xcb_setup_roots_iterator(setup);
    screen = iterator.data;

    width = screen->width_in_pixels;
    height = screen->height_in_pixels;

    key_symbols = xcb_key_symbols_alloc(connection);
    assert(key_symbols != NULL);

    uint32_t event_mask = 0;
    event_mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
    uint32_t event_flags[2];
    event_flags[0] = screen->black_pixel;
    event_flags[1] = XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE
        | XCB_EVENT_MASK_KEY_PRESS | XCB_EVENT_MASK_KEY_RELEASE
        | XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_POINTER_MOTION
        | XCB_EVENT_MASK_ENTER_WINDOW | XCB_EVENT_MASK_LEAVE_WINDOW
        | XCB_EVENT_MASK_FOCUS_CHANGE;
    window = xcb_generate_id(connection);
    xcb_create_window(
        connection,
        XCB_COPY_FROM_PARENT,
        window,
        screen->root,
        0,
        0,
        width,
        height,
        10,
        XCB_WINDOW_CLASS_INPUT_OUTPUT,
        screen->root_visual,
        event_mask,
        event_flags
    );

    // Verify the window was created (optional).
    xcb_get_window_attributes_cookie_t attr_cookie =
        xcb_get_window_attributes(connection, window);
    xcb_get_window_attributes_reply_t* attr_reply =
        xcb_get_window_attributes_reply(connection, attr_cookie, NULL);

    if (!attr_reply) {
        fprintf(stderr, "Failed to query window - maybe it wasn't created.\n");
    } else {
        printf("Window created successfully and is valid.\n");
        free(attr_reply);
    }
    // Map the window on the screen.
    xcb_map_window(connection, window);
    // Make sure commands are sent before we pause so that the window gets shown.
    xcb_flush(connection);
    HideCursor();
}

void WindowXCBVulkan::configureWindow() {
    uint16_t mask = XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y
        | XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT;
    const uint32_t values[] = {
        320, // x.
        180, // y.
        width,
        height
    };

    xcb_configure_window(connection, window, mask, values);
    xcb_flush(connection);
}

void WindowXCBVulkan::HideCursor() {
    xcb_pixmap_t foreground_pixmap_id = xcb_generate_id(connection);
    xcb_create_pixmap(connection, 1, foreground_pixmap_id, window, 8, 8);

    // Create graphical context.
    xcb_gcontext_t graphical_context = xcb_generate_id(connection);

    uint32_t mask = XCB_GC_FOREGROUND | XCB_GC_BACKGROUND;
    uint32_t values_list[2];
    values_list[0] = screen->black_pixel;
    values_list[1] = screen->white_pixel;

    xcb_create_gc(
        connection,
        graphical_context,
        window,
        XCB_GC_FOREGROUND | XCB_GC_BACKGROUND,
        values_list
    );

    const uint8_t pix_map_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                    0x00, 0x00, 0x00, 0x00};

    xcb_put_image(
        connection,
        XCB_IMAGE_FORMAT_XY_PIXMAP,
        foreground_pixmap_id,
        graphical_context,
        0,
        0,
        100,
        100,
        0,
        8,
        32,
        pix_map_data
    );

    xcb_cursor_t cursor = xcb_generate_id(connection);
    xcb_create_cursor(
        connection,
        cursor,
        foreground_pixmap_id,
        foreground_pixmap_id,
        0,
        0,
        0,
        0,
        0,
        0,
        8,
        8
    );

    mask = XCB_CW_CURSOR;
    uint32_t value_list = cursor;
    xcb_change_window_attributes(connection, window, mask, &value_list);

    xcb_free_cursor(connection, cursor);
}

xcb_connection_t* WindowXCBVulkan::GetConnection() {
    return connection;
}

uint32_t WindowXCBVulkan::GetWindow() {
    return window;
}

void WindowXCBVulkan::Disconnect() {
    xcb_disconnect(connection);
}

void WindowXCBVulkan::SwapBuffers() {};

void WindowXCBVulkan::ClearDisplay() {};

void WindowXCBVulkan::print_modifiers(uint32_t mask) {
    const char **mod,
        *mods[] = {
            "Shift",
            "Lock",
            "Ctrl",
            "Alt",
            "Mod2",
            "Mod3",
            "Mod4",
            "Mod5",
            "Button1",
            "Button2",
            "Button3",
            "Button4",
            "Button5"
        };
    printf("Modifier mask: ");
    for (mod = mods; mask; mask >>= 1, mod++) {
        if (mask & 1) {
            std::cout << *mod << std::endl;
        }
    };
    putchar('\n');
}

bool WindowXCBVulkan::HandleEvent([[maybe_unused]] CEvent& _Event) {
    xcb_generic_event_t* generic_event;
    bool next_generic_event_flag = false;
    while (next_generic_event_flag
           || (generic_event = xcb_poll_for_event(connection))) {
        next_generic_event_flag = false;
        switch (generic_event->response_type & ~0x80) {
            case XCB_EXPOSE: {
                [[maybe_unused]] xcb_expose_event_t* expose_event =
                    (xcb_expose_event_t*)generic_event;

                printf(
                    "Window %i exposed. Region to be redrawn at location (%d,%d), with dimension (%d,%d)\n",
                    expose_event->window,
                    expose_event->x,
                    expose_event->y,
                    expose_event->width,
                    expose_event->height
                );

                if (!isWindowResizeRead) {
                    width = expose_event->width;
                    height = expose_event->height;
                    isWindowResizeRead = true;
                }
                break;
            }
            case XCB_BUTTON_PRESS: {
                xcb_button_press_event_t* expose_event =
                    (xcb_button_press_event_t*)generic_event;
                switch (expose_event->detail) {
                    case 1:
                        _Event.SetEvent(EEvents::eMOUSE_LEFT_BUTTON);
                        break;
                    case 3:
                        _Event.SetEvent(EEvents::eMOUSE_RIGHT_BUTTON);
                        break;
                    case 4:
                        break;
                    case 5:
                        break;
                }

                break;
            }
            case XCB_BUTTON_RELEASE: {
                xcb_button_release_event_t* expose_event =
                    (xcb_button_release_event_t*)generic_event;
                switch (expose_event->detail) {
                    case 1:
                        _Event.SetEvent(EEvents::eMOUSE_LEFT_BUTTON_RELEASE);
                        _Event.isLeftMouseButtonReleased = true;
                        break;
                    case 3:
                        _Event.SetEvent(EEvents::eMOUSE_RIGHT_BUTTON_RELEASE);
                        break;
                }

                break;
            }
            case XCB_MOTION_NOTIFY: {
                xcb_motion_notify_event_t* expose_event =
                    (xcb_motion_notify_event_t*)generic_event;

                _Event.SetEvent(EEvents::eMOUSE_POINTER_POSITION);
                _Event.mousePointerPosition.position_X = expose_event->event_x;
                _Event.mousePointerPosition.position_Y = expose_event->event_y;
            }
            case XCB_MAP_WINDOW: {
                // Make sure commands are sent before we pause so that the
                // window gets shown.
                xcb_flush(connection);

                xcb_grab_pointer_cookie_t cookie = xcb_grab_pointer(
                    connection,
                    1,
                    window,
                    XCB_EVENT_MASK_POINTER_MOTION | XCB_EVENT_MASK_BUTTON_PRESS,
                    XCB_GRAB_MODE_ASYNC,
                    XCB_GRAB_MODE_ASYNC,
                    window,
                    XCB_NONE,
                    XCB_CURRENT_TIME
                );
                xcb_grab_pointer_reply_t* grab_pointer_reply =
                    xcb_grab_pointer_reply(connection, cookie, NULL);
                free(grab_pointer_reply);
                break;
            }
            case XCB_ENTER_NOTIFY: {
                [[maybe_unused]] xcb_enter_notify_event_t* expose_event =
                    (xcb_enter_notify_event_t*)generic_event;
                break;
            }
            case XCB_FOCUS_IN:
                isFocused = true;
                xcb_grab_pointer(
                    connection,
                    1,
                    window,
                    XCB_EVENT_MASK_POINTER_MOTION | XCB_EVENT_MASK_BUTTON_PRESS,
                    XCB_GRAB_MODE_ASYNC,
                    XCB_GRAB_MODE_ASYNC,
                    window,
                    XCB_NONE,
                    XCB_CURRENT_TIME
                );
                break;
            case XCB_FOCUS_OUT:
                isFocused = false;
                xcb_ungrab_pointer(connection, XCB_CURRENT_TIME);
                xcb_flush(connection);
                break;
            case XCB_KEY_PRESS: {
                xcb_key_press_event_t* expose_event =
                    (xcb_key_press_event_t*)generic_event;
                xcb_keysym_t keysym =
                    xcb_key_press_lookup_keysym(key_symbols, expose_event, 0);
                switch (keysym) {
                    case 65307:
                        _Event.SetEvent(EEvents::eGAME_LOOP_KILL);
                        break;
                    case 105:
                        _Event.SetEvent(EEvents::eINVENTORY);
                        break;
                    case 97:
                        _Event.SetEvent(EEvents::eMOVE_LEFT);
                        break;
                    case 100:
                        _Event.SetEvent(EEvents::eMOVE_RIGHT);
                        break;
                    case 115:
                        _Event.SetEvent(EEvents::eMOVE_BACKWARD);
                        break;
                    case 119:
                        _Event.SetEvent(EEvents::eMOVE_FORWARD);
                        break;
                    case 32:
                        _Event.SetEvent(EEvents::eJUMP);
                        break;
                }

                break;
            }
            case XCB_KEY_RELEASE: {
                xcb_key_release_event_t* key_release_event =
                    (xcb_key_release_event_t*)generic_event;
                next_generic_event = xcb_poll_for_event(connection);
                if (next_generic_event != NULL) {
                    xcb_key_press_event_t* key_press_event =
                        (xcb_key_press_event_t*)next_generic_event;
                    xcb_keysym_t press_keysym = xcb_key_press_lookup_keysym(
                        key_symbols,
                        key_press_event,
                        0
                    );
                    xcb_keysym_t release_keysym = xcb_key_press_lookup_keysym(
                        key_symbols,
                        key_release_event,
                        0
                    );

                    if (next_generic_event->response_type == XCB_KEY_PRESS
                        && key_press_event->time == key_release_event->time
                        && press_keysym == release_keysym) {
                        free(generic_event);
                        generic_event = NULL;
                        free(next_generic_event);
                        next_generic_event = NULL;
                        continue;
                    } else {
                        next_generic_event_flag = true;
                    }
                }

                xcb_keysym_t release_keysym = xcb_key_press_lookup_keysym(
                    key_symbols,
                    key_release_event,
                    0
                );
                switch (release_keysym) {
                    case 105:
                        _Event.SetEvent(glvm::core::eINVENTORY_RELEASE);
                        break;
                    case 97:
                        _Event.SetEvent(glvm::core::eKEYRELEASE_A);
                        break;
                    case 100:
                        _Event.SetEvent(glvm::core::eKEYRELEASE_D);
                        break;
                    case 115:
                        _Event.SetEvent(glvm::core::eKEYRELEASE_S);
                        break;
                    case 119:
                        _Event.SetEvent(glvm::core::eKEYRELEASE_W);
                        break;
                    case 32:
                        _Event.SetEvent(glvm::core::eKEYRELEASE_JUMP);
                        break;
                }

                break;
            }
        }
        if (next_generic_event != NULL) {
            *generic_event = *next_generic_event;
            free(next_generic_event);
            next_generic_event = NULL;
        } else {
            free(generic_event);
        }
        Input_Stack_.ControlInput(_Event);
    }
    isWindowResizeRead = false;
    return false;
};

void WindowXCBVulkan::Close() {
    xcb_key_symbols_free(key_symbols);
    xcb_disconnect(connection);
};

void WindowXCBVulkan::CursorLock(
    [[maybe_unused]] int _x_position,
    [[maybe_unused]] int _y_position,
    [[maybe_unused]] int* _x_offset,
    [[maybe_unused]] int* _y_offset
) {
    int iOffset_X = 0, iOffset_Y = 0;
    iOffset_X = _x_position - (int)(width / 2);
    iOffset_Y = _y_position - (int)(height / 2);

    *_x_offset += iOffset_X;
    *_y_offset -= iOffset_Y;
    xcb_warp_pointer(
        connection,
        XCB_NONE,
        window,
        0,
        0,
        0,
        0,
        (int)(width / 2),
        (int)(height / 2)
    );
    xcb_flush(connection);
};
} // namespace glvm::core
