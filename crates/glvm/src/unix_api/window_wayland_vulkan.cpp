#include "glvm/unix_api/window_wayland_vulkan.hpp"

#include "glvm/graphic_api/vulkan.hpp"
#include "glvm/unix_api/pointer_constraints_unstable_v1_client_protocol.h"

#include <vulkan/vulkan_core.h>
#include <wayland-client-protocol.h>
#include <wayland-util.h>

namespace glvm::core {
static WindowWaylandVulkan windowWaylandVulkan;

WindowWaylandVulkan::WindowWaylandVulkan() {}

void WindowWaylandVulkan::init() {
    display = wl_display_connect(0);
    registry = wl_display_get_registry(display);
    wl_registry_add_listener(
        registry,
        &registry_listener,
        (void*)(&windowWaylandVulkan)
    );
    wl_display_roundtrip(display);
    if (!compositor || !xdg_shell) {
        fprintf(stderr, "Error: compositor or xdg_shell is NULL!\n");
        exit(1);
    }
    wl_surface = wl_compositor_create_surface(compositor);
    pointer_surface = wl_compositor_create_surface(compositor);
    frame_callback = wl_surface_frame(wl_surface);
    wl_callback_add_listener(
        frame_callback,
        &callback_listener,
        (void*)(&windowWaylandVulkan)
    );
    xdg_surface = xdg_wm_base_get_xdg_surface(xdg_shell, wl_surface);
    xdg_surface_add_listener(
        xdg_surface,
        &xdg_surface_listener,
        (void*)(&windowWaylandVulkan)
    );
    xdg_topLevel = xdg_surface_get_toplevel(xdg_surface);
    xdg_toplevel_add_listener(
        xdg_topLevel,
        &xdg_toplevel_listener,
        (void*)(&windowWaylandVulkan)
    );
    xdg_toplevel_set_title(xdg_topLevel, "wayland glvm client");
    wl_surface_commit(wl_surface);
}

bool WindowWaylandVulkan::HandleEvent([[maybe_unused]] CEvent& _Event) {
    _Event.mousePointerPosition.position_X = x_pointer;
    _Event.mousePointerPosition.position_Y = y_pointer;
    x_pointer = 0;
    y_pointer = 0;
    wl_display_dispatch(display);
    return false;
}

// Create transparent cursor.
struct wl_buffer* WindowWaylandVulkan::create_transparent_cursor(
    [[maybe_unused]] struct wl_shm* shm
) {
    int size = 4 * 64 * 64; // 64x64 RGBA cursor (common size).
    int32_t file_descriptor = alocate_shared_memory(size);
    void* data = mmap(
        NULL,
        width * height * 4,
        PROT_READ | PROT_WRITE,
        MAP_SHARED,
        file_descriptor,
        0
    );

    // Fill with transparent pixels.
    for (int i = 0; i < 64 * 64; ++i) {
        ((int*)data)[i] = 0x00000000;
    }

    struct wl_shm_pool* pool = wl_shm_create_pool(shm, file_descriptor, size);
    struct wl_buffer* buffer = wl_shm_pool_create_buffer(
        pool,
        0,
        64,
        64,
        64 * 4,
        WL_SHM_FORMAT_ARGB8888
    );

    munmap(data, size);
    close(file_descriptor);
    wl_shm_pool_destroy(pool);

    return buffer;
}

void WindowWaylandVulkan::SwapBuffers() {};
void WindowWaylandVulkan::ClearDisplay() {};

void WindowWaylandVulkan::CursorLock(
    [[maybe_unused]] int _x_position,
    [[maybe_unused]] int _y_position,
    [[maybe_unused]] int* _x_offset,
    [[maybe_unused]] int* _y_offset
) {
    static int flag = 0;
    if (flag == 0) {
        *_x_offset = -((int)width / 2);
        *_y_offset = -((int)height / 2);
        ++flag;
    } else {
        *_x_offset = _x_position;
        *_y_offset = _y_position;
    }
};

void WindowWaylandVulkan::Close() {
    if (keyboard) {
        wl_keyboard_destroy(keyboard);
    }
    // Release a Wayland seat object, which is responsible for managing input
    // devices like keyboards, mice, or touchscreens.
    wl_seat_release(seat);
    if (buffer) {
        wl_buffer_destroy(buffer);
    }
    xdg_toplevel_destroy(xdg_topLevel);
    xdg_surface_destroy(xdg_surface);
    wl_surface_destroy(wl_surface);
    wl_display_disconnect(display);
}

int32_t alocate_shared_memory(uint64_t size) {
    char name[8];
    name[0] = '/';
    name[7] = 0;
    for (int8_t i = 1; i < 6; ++i) {
        name[i] = (rand() & 23) + 97;
    }
    // shm_open, shm_unlink - create/open or unlink POSIX shared memory objects.
    int32_t file_descriptor = shm_open(
        name,
        O_RDWR | O_CREAT | O_EXCL,
        S_IWUSR | S_IRUSR | S_IWOTH | S_IROTH
    );
    shm_unlink(name);
    // File truncation means cutting off a file at a certain size - either
    // shrinking it or expanding it.
    [[maybe_unused]] int result = ftruncate(file_descriptor, size);

    return file_descriptor;
}

void resize(void* data) {
    WindowWaylandVulkan* resizeData = (WindowWaylandVulkan*)data;

    int32_t file_descriptor =
        alocate_shared_memory(resizeData->width * resizeData->height * 4);

    // 1. addr: Preferred memory address where mapping should start.
    // 2. length: how many bytes to map.
    // 3. prot: memory protection (read/write/exec).
    // 4. flags: type of mapping (private/shared/anonymous, etc.).
    // 5. fd: file descriptor to map.
    // 6. offset: offset into file (usually 0 to map from the beginning).
    // When you pass 0 (or NULL) as the first argument, you're telling the
    // kernel: "I don't care where you map the file in memory - just choose a
    // suitable address for me."
    resizeData->pixels = mmap(
        0,
        resizeData->width * resizeData->height * 4,
        PROT_READ | PROT_WRITE,
        MAP_SHARED,
        file_descriptor,
        0
    );

    // Create a shared memory pool that clients can use to allocate memory
    // buffers for drawing.
    struct wl_shm_pool* pool = wl_shm_create_pool(
        resizeData->shared_memory,
        file_descriptor,
        resizeData->width * resizeData->height * 4
    );
    // It tells Wayland: "Take this part of the memory pool and treat it as an
    // image buffer that I'll draw onto a window."
    resizeData->buffer = wl_shm_pool_create_buffer(
        pool,
        0,
        resizeData->width,
        resizeData->height,
        resizeData->width * 4,
        WL_SHM_FORMAT_ABGR8888
    );
    wl_shm_pool_destroy(pool);
    close(file_descriptor);
}

void draw(void* data) {
    WindowWaylandVulkan* drawData = (WindowWaylandVulkan*)data;

    // Fill a block of memory with a specific byte value.
    memset(
        drawData->pixels,
        drawData->constant_byte,
        drawData->width * drawData->height * 4
    );

    // Set a buffer as the content of this surface.
    wl_surface_attach(drawData->wl_surface, drawData->buffer, 0, 0);
    // Mark a specific area of a Wayland surface as "damaged", which means the
    // compositor should re-render or update that area.
    wl_surface_damage_buffer(
        drawData->wl_surface,
        0,
        0,
        drawData->width,
        drawData->height
    );
    // Commit the changes you've made to a Wayland surface, sending them to the
    // compositor so they can be applied (i.e., rendered to the screen).
    wl_surface_commit(drawData->wl_surface);
}

void xdg_toplevel_configure(
    [[maybe_unused]] void* data,
    [[maybe_unused]] struct xdg_toplevel* xdg_toplevel,
    int32_t new_width,
    int32_t new_height,
    [[maybe_unused]] struct wl_array* atate
) {
    if (!new_width && !new_height) {
        return;
    }

    WindowWaylandVulkan* xdg_topLevelData = (WindowWaylandVulkan*)data;

    if (xdg_topLevelData->width != new_width
        || xdg_topLevelData->height != new_height) {
        munmap(
            xdg_topLevelData->pixels,
            xdg_topLevelData->width * new_height * 4
        );
        xdg_topLevelData->width = new_width;
        xdg_topLevelData->height = new_height;
        resize(data);
    }
}

void xdg_toplevel_close(
    [[maybe_unused]] void* data,
    [[maybe_unused]] struct xdg_toplevel* xdg_toplevel
) {
    WindowWaylandVulkan* xdg_topLevelData = (WindowWaylandVulkan*)data;

    xdg_topLevelData->close_xdg_toplevel = 1;
}

void xdg_surface_configure(
    [[maybe_unused]] void* data,
    struct xdg_surface* xdg_surface,
    uint32_t serial
) {
    // Acknowledge a configure event sent by the Wayland compositor to your
    // xdg_surface In Wayland, when the compositor wants to change your window
    // (like resizing it), it sends a configure event to your surface. You must
    // call xdg_surface_ack_configure() to confirm that you received and
    // accepted this change. If you don't call it, your window won't be shown or
    // updated properly.
    WindowWaylandVulkan* xdg_surfaceConfigData = (WindowWaylandVulkan*)data;

    xdg_surface_ack_configure(xdg_surface, serial);
    if (!xdg_surfaceConfigData->pixels) {
        resize(data);
    }
}

void new_frame(
    [[maybe_unused]] void* data,
    struct wl_callback* frame_call_back,
    [[maybe_unused]] uint32_t callback_data
) {
    WindowWaylandVulkan* xdg_surfaceConfigData = (WindowWaylandVulkan*)data;

    wl_callback_destroy(frame_call_back);
    frame_call_back = wl_surface_frame(xdg_surfaceConfigData->wl_surface);
    wl_callback_add_listener(
        frame_call_back,
        &windowWaylandVulkan.callback_listener,
        data
    );
}

void shell_ping(
    [[maybe_unused]] void* data,
    struct xdg_wm_base* shell,
    uint32_t serial
) {
    xdg_wm_base_pong(shell, serial);
}

void keyboard_keymap(
    [[maybe_unused]] void* data,
    [[maybe_unused]] struct wl_keyboard* keyboard,
    [[maybe_unused]] uint32_t format,
    [[maybe_unused]] int32_t keymap_file_descriptor,
    [[maybe_unused]] uint32_t size
) {}

void keyboard_enter(
    void* data,
    [[maybe_unused]] struct wl_keyboard* keyboard,
    [[maybe_unused]] uint32_t serial,
    [[maybe_unused]] struct wl_surface* surface,
    [[maybe_unused]] struct wl_array* keys
) {
    WindowWaylandVulkan* waylandWindow = (WindowWaylandVulkan*)data;
    waylandWindow->isFocused = true;
}

void keyboard_leave(
    void* data,
    [[maybe_unused]] struct wl_keyboard* keyboard,
    [[maybe_unused]] uint32_t serial,
    [[maybe_unused]] struct wl_surface* surface
) {
    WindowWaylandVulkan* waylandWindow = (WindowWaylandVulkan*)data;
    waylandWindow->isFocused = false;
}

void keyboard_key(
    [[maybe_unused]] void* data,
    [[maybe_unused]] struct wl_keyboard* keyboard,
    [[maybe_unused]] uint32_t serial,
    [[maybe_unused]] uint32_t time,
    uint32_t key,
    [[maybe_unused]] uint32_t state
) {
    if (state == WL_KEYBOARD_KEY_STATE_PRESSED) {
        if (key == 1) {
            g_eEvent.SetEvent(EEvents::eGAME_LOOP_KILL);
            Input_Stack_.ControlInput(g_eEvent);
        }
        if (key == 17) {
            g_eEvent.SetEvent(EEvents::eMOVE_FORWARD);
            Input_Stack_.ControlInput(g_eEvent);
        }
        if (key == 31) {
            g_eEvent.SetEvent(EEvents::eMOVE_BACKWARD);
            Input_Stack_.ControlInput(g_eEvent);
        }
        if (key == 30) {
            g_eEvent.SetEvent(EEvents::eMOVE_LEFT);
            Input_Stack_.ControlInput(g_eEvent);
        }
        if (key == 32) {
            g_eEvent.SetEvent(EEvents::eMOVE_RIGHT);
            Input_Stack_.ControlInput(g_eEvent);
        }
        if (key == 57) {
            g_eEvent.SetEvent(EEvents::eJUMP);
            Input_Stack_.ControlInput(g_eEvent);
        }
        if (key == 23) {
            g_eEvent.SetEvent(EEvents::eINVENTORY);
            Input_Stack_.ControlInput(g_eEvent);
        }
    }

    if (state == WL_KEYBOARD_KEY_STATE_RELEASED) {
        if (key == 1) {
            WindowWaylandVulkan* registryListenerData =
                (WindowWaylandVulkan*)data;
            wl_display_disconnect(registryListenerData->display);
        }
        if (key == 17) {
            g_eEvent.SetEvent(EEvents::eKEYRELEASE_W);
            Input_Stack_.ControlInput(g_eEvent);
        }
        if (key == 31) {
            g_eEvent.SetEvent(EEvents::eKEYRELEASE_S);
            Input_Stack_.ControlInput(g_eEvent);
        }
        if (key == 30) {
            g_eEvent.SetEvent(EEvents::eKEYRELEASE_A);
            Input_Stack_.ControlInput(g_eEvent);
        }
        if (key == 32) {
            g_eEvent.SetEvent(EEvents::eKEYRELEASE_D);
            Input_Stack_.ControlInput(g_eEvent);
        }
        if (key == 57) {
            g_eEvent.SetEvent(EEvents::eKEYRELEASE_JUMP);
            Input_Stack_.ControlInput(g_eEvent);
        }
        if (key == 23) {
            g_eEvent.SetEvent(EEvents::eINVENTORY_RELEASE);
            Input_Stack_.ControlInput(g_eEvent);
        }
    }
}

void keyboard_modifiers(
    [[maybe_unused]] void* data,
    [[maybe_unused]] struct wl_keyboard* keyboard,
    [[maybe_unused]] uint32_t serial,
    [[maybe_unused]] uint32_t mods_depressed,
    [[maybe_unused]] uint32_t mods_latched,
    [[maybe_unused]] uint32_t mods_locked,
    [[maybe_unused]] uint32_t group
) {}

void keyboard_repeat_info(
    [[maybe_unused]] void* data,
    [[maybe_unused]] struct wl_keyboard* keyboard,
    [[maybe_unused]] int32_t rate,
    [[maybe_unused]] int32_t delay
) {}

// Pointer listener callbacks.
void pointer_enter(
    [[maybe_unused]] void* data,
    [[maybe_unused]] struct wl_pointer* pointer,
    [[maybe_unused]] uint32_t serial,
    [[maybe_unused]] struct wl_surface* surface,
    [[maybe_unused]] wl_fixed_t sx,
    [[maybe_unused]] wl_fixed_t sy
) {}

void pointer_leave(
    [[maybe_unused]] void* data,
    [[maybe_unused]] struct wl_pointer* pointer,
    [[maybe_unused]] uint32_t serial,
    [[maybe_unused]] struct wl_surface* surface
) {}

void pointer_motion(
    [[maybe_unused]] void* data,
    [[maybe_unused]] struct wl_pointer* pointer,
    [[maybe_unused]] uint32_t time,
    [[maybe_unused]] wl_fixed_t sx,
    [[maybe_unused]] wl_fixed_t sy
) {}

void pointer_axis(
    [[maybe_unused]] void* data,
    [[maybe_unused]] struct wl_pointer* pointer,
    [[maybe_unused]] uint32_t time,
    [[maybe_unused]] uint32_t axis,
    [[maybe_unused]] wl_fixed_t value
) {}

void pointer_button(
    [[maybe_unused]] void* data,
    [[maybe_unused]] struct wl_pointer* pointer,
    [[maybe_unused]] uint32_t serial,
    [[maybe_unused]] uint32_t time,
    uint32_t button,
    uint32_t state
) {
    if (state == WL_POINTER_BUTTON_STATE_PRESSED) {
        if (button == 272) {
            g_eEvent.SetEvent(EEvents::eMOUSE_LEFT_BUTTON);
            Input_Stack_.ControlInput(g_eEvent);
        }
    }
    if (state == WL_POINTER_BUTTON_STATE_RELEASED) {
        if (button == 272) {
            g_eEvent.SetEvent(EEvents::eMOUSE_LEFT_BUTTON_RELEASE);
            Input_Stack_.ControlInput(g_eEvent);
            g_eEvent.isLeftMouseButtonReleased = true;
        }
    }
    // Hide cursor on first opportunity.
    if (!windowWaylandVulkan.hideAndLockPointer) {
        WindowWaylandVulkan* registryListenerData = (WindowWaylandVulkan*)data;
        windowWaylandVulkan.hideAndLockPointer = true;
        struct wl_buffer* transparent =
            windowWaylandVulkan.create_transparent_cursor(
                registryListenerData->pointer_shared_memory
            );
        wl_surface_attach(
            registryListenerData->pointer_surface,
            transparent,
            0,
            0
        );
        wl_surface_commit(registryListenerData->pointer_surface);
        wl_pointer_set_cursor(
            pointer,
            serial,
            registryListenerData->pointer_surface,
            0,
            0
        );

        if (!registryListenerData->pointer_constraints) {
            return;
        }

        // Lock pointer to main window surface, not pointer_surface.
        [[maybe_unused]] zwp_locked_pointer_v1* locked_pointer =
            zwp_pointer_constraints_v1_lock_pointer(
                registryListenerData->pointer_constraints,
                registryListenerData->wl_surface,
                pointer,
                NULL,
                ZWP_POINTER_CONSTRAINTS_V1_LIFETIME_PERSISTENT
            );

        // Get relative motion.
        registryListenerData->relative_pointer =
            zwp_relative_pointer_manager_v1_get_relative_pointer(
                registryListenerData->relative_pointer_manager,
                pointer
            );
        zwp_relative_pointer_v1_add_listener(
            registryListenerData->relative_pointer,
            &windowWaylandVulkan.relative_pointer_listener,
            NULL
        );
    }
}

void handle_relative_motion(
    [[maybe_unused]] void* data,
    [[maybe_unused]] struct zwp_relative_pointer_v1* rel_pointer,
    [[maybe_unused]] uint32_t utime_hi,
    [[maybe_unused]] uint32_t utime_lo,
    wl_fixed_t dx,
    wl_fixed_t dy,
    [[maybe_unused]] wl_fixed_t dx_unaccel,
    [[maybe_unused]] wl_fixed_t dy_unaccel
) {
    x_pointer = wl_fixed_to_int(dx);
    y_pointer = wl_fixed_to_int(dy);
}

void seat_capabilities(
    [[maybe_unused]] void* data,
    struct wl_seat* seat,
    uint32_t capabilities
) {
    WindowWaylandVulkan* registryListenerData = (WindowWaylandVulkan*)data;
    // Handle pointer capabilities.
    if ((capabilities & WL_SEAT_CAPABILITY_POINTER)
        && !registryListenerData->pointer) {
        registryListenerData->pointer = wl_seat_get_pointer(seat);
        wl_pointer_add_listener(
            registryListenerData->pointer,
            &windowWaylandVulkan.pointer_listener,
            data
        );
    } else if (
        !(capabilities & WL_SEAT_CAPABILITY_POINTER)
        && registryListenerData->pointer
    ) {
        wl_pointer_destroy(registryListenerData->pointer);
        registryListenerData->pointer = NULL;
    }

    if (capabilities & WL_SEAT_CAPABILITY_KEYBOARD
        && !registryListenerData->keyboard) {
        registryListenerData->keyboard = wl_seat_get_keyboard(seat);
        wl_keyboard_add_listener(
            registryListenerData->keyboard,
            &windowWaylandVulkan.keyboard_listener,
            data
        );
    }
}

void seat_name(
    [[maybe_unused]] void* data,
    [[maybe_unused]] struct wl_seat* seat,
    [[maybe_unused]] const char* name
) {}

static void output_geometry(
    [[maybe_unused]] void* data,
    [[maybe_unused]] struct wl_output* output,
    [[maybe_unused]] int32_t x,
    [[maybe_unused]] int32_t y,
    [[maybe_unused]] int32_t physical_width,
    [[maybe_unused]] int32_t physical_height,
    [[maybe_unused]] int32_t subpixel,
    [[maybe_unused]] const char* make,
    [[maybe_unused]] const char* model,
    [[maybe_unused]] int32_t transform
) {}

static void output_mode(
    void* data,
    [[maybe_unused]] struct wl_output* output,
    uint32_t flags,
    int32_t width,
    int32_t height,
    [[maybe_unused]] int32_t refresh
) {
    if (flags & WL_OUTPUT_MODE_CURRENT) {
        WindowWaylandVulkan* waylandWindow = (WindowWaylandVulkan*)data;
        waylandWindow->width = width;
        waylandWindow->height = height;
    }
}

static void output_done(
    [[maybe_unused]] void* data,
    [[maybe_unused]] struct wl_output* output
) {}

void registry_global(
    [[maybe_unused]] void* data,
    struct wl_registry* registry,
    uint32_t name,
    const char* interface,
    [[maybe_unused]] uint32_t version
) {
    WindowWaylandVulkan* registryListenerData = (WindowWaylandVulkan*)data;

    if (!strcmp(interface, wl_compositor_interface.name)) {
        registryListenerData->compositor = (wl_compositor*)
            wl_registry_bind(registry, name, &wl_compositor_interface, 4);
    } else if (!strcmp(interface, wl_shm_interface.name)) {
        registryListenerData->shared_memory =
            (wl_shm*)wl_registry_bind(registry, name, &wl_shm_interface, 1);
        registryListenerData->pointer_shared_memory =
            (wl_shm*)wl_registry_bind(registry, name, &wl_shm_interface, 1);
    } else if (!strcmp(interface, zwp_pointer_constraints_v1_interface.name)) {
        registryListenerData->pointer_constraints =
            (zwp_pointer_constraints_v1*)wl_registry_bind(
                registry,
                name,
                &zwp_pointer_constraints_v1_interface,
                1
            );
    } else if (!strcmp(
                   interface,
                   zwp_relative_pointer_manager_v1_interface.name
               )) {
        registryListenerData->relative_pointer_manager =
            (zwp_relative_pointer_manager_v1*)wl_registry_bind(
                registry,
                name,
                &zwp_relative_pointer_manager_v1_interface,
                1
            );
    } else if (!strcmp(interface, xdg_wm_base_interface.name)) {
        registryListenerData->xdg_shell = (xdg_wm_base*)
            wl_registry_bind(registry, name, &xdg_wm_base_interface, 1);
        xdg_wm_base_add_listener(
            registryListenerData->xdg_shell,
            &windowWaylandVulkan.shell_listener,
            0
        );
    } else if (!strcmp(interface, wl_seat_interface.name)) {
        registryListenerData->seat =
            (wl_seat*)wl_registry_bind(registry, name, &wl_seat_interface, 1);
        wl_seat_add_listener(
            registryListenerData->seat,
            &windowWaylandVulkan.seat_lintener,
            data
        );
    } else if (!strcmp(interface, wl_output_interface.name)) {
        struct wl_output* output = (wl_output*)
            wl_registry_bind(registry, name, &wl_output_interface, 1);
        wl_output_add_listener(
            output,
            &windowWaylandVulkan.output_listener,
            data
        );
    }
}

void registry_global_remove(
    [[maybe_unused]] void* data,
    [[maybe_unused]] struct wl_registry* registry,
    [[maybe_unused]] uint32_t name
) {}

WindowWaylandVulkan* initializeWaylandWindow() {
    windowWaylandVulkan.xdg_toplevel_listener = {
        .configure = xdg_toplevel_configure,
        .close = xdg_toplevel_close,
        .configure_bounds = nullptr,
        .wm_capabilities = nullptr
    };
    windowWaylandVulkan.xdg_surface_listener = {
        .configure = xdg_surface_configure
    };
    windowWaylandVulkan.callback_listener = {
        // Notify the client when the related request is done. param
        // callback_data request-specific data for the callback
        .done = new_frame
    };
    windowWaylandVulkan.shell_listener = {.ping = shell_ping};
    windowWaylandVulkan.output_listener =
        {.geometry = output_geometry, .mode = output_mode, .done = output_done};
    windowWaylandVulkan.keyboard_listener = {
        .keymap = keyboard_keymap,
        .enter = keyboard_enter,
        .leave = keyboard_leave,
        .key = keyboard_key,
        .modifiers = keyboard_modifiers,
        .repeat_info = keyboard_repeat_info
    };

    windowWaylandVulkan.relative_pointer_listener = {
        .relative_motion = handle_relative_motion
    };
    windowWaylandVulkan.pointer_listener = {
        .enter = pointer_enter,
        .leave = pointer_leave,
        .motion = pointer_motion,
        .button = pointer_button,
        .axis = pointer_axis,
        .frame = nullptr,
        .axis_source = nullptr,
        .axis_stop = nullptr,
        .axis_discrete = nullptr,
        .axis_value120 = nullptr,
        .axis_relative_direction = nullptr
    };
    windowWaylandVulkan.seat_lintener = {
        .capabilities = seat_capabilities,
        .name = seat_name
    };
    windowWaylandVulkan.registry_listener = {
        .global = registry_global,
        .global_remove = registry_global_remove
    };

    windowWaylandVulkan.init();
    return &windowWaylandVulkan;
}
}; // namespace glvm::core
