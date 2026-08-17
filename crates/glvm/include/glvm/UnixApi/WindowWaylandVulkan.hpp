#ifdef __linux__
#ifndef WINDOW_WAYLAND_VULKAN
#define WINDOW_WAYLAND_VULKAN

#include "glvm/EventsStack.hpp"
#include "glvm/Globals.hpp"
#include "glvm/IWindow.hpp"
#include "glvm/UnixApi/pointer-constraints-unstable-v1-client-protocol.h"
#include "glvm/UnixApi/relative-pointer-unstable-v1-client-protocol.h"
#include "glvm/UnixApi/xdg-shell-client-protocol.h"

#include <X11/Xlib.h>
#include <cstdint>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <wayland-client-core.h>
#include <wayland-client-protocol.h>
#include <wayland-client.h>

namespace GLVM::core {
struct WindowWaylandVulkan: IWindow {
    WindowWaylandVulkan();
    void init();
    void Close() override;
    bool HandleEvent(CEvent& _Event) override;
    static int create_anonymous_file(off_t size);
    struct wl_buffer* create_transparent_cursor(struct wl_shm* shm);
    void SwapBuffers() override;
    void ClearDisplay() override;
    void CursorLock(
        int _x_position,
        int _y_position,
        int* _x_offset,
        int* _y_offset
    ) override;

    //		CStack           * Input_Stack_;
    int previous_X = 960;
    int previous_Y = 540;
    bool hideAndLockPointer = false;
    // int previous_X = 0;
    // int previous_Y = 0;
    struct xdg_toplevel_listener xdg_toplevel_listener;
    struct xdg_surface_listener xdg_surface_listener;
    struct wl_callback_listener callback_listener;
    struct xdg_wm_base_listener shell_listener;
    struct wl_keyboard_listener keyboard_listener;
    struct zwp_relative_pointer_v1_listener relative_pointer_listener;
    struct wl_pointer_listener pointer_listener;
    struct wl_seat_listener seat_lintener;
    struct wl_registry_listener registry_listener;

    struct wl_surface* wl_surface;
    struct wl_compositor* compositor;
    struct xdg_toplevel* xdg_topLevel;
    struct xdg_wm_base* xdg_shell;
    struct wl_buffer* buffer;
    struct wl_shm* shared_memory;
    struct wl_seat* seat;
    struct wl_keyboard* keyboard;
    struct wl_pointer* pointer;
    struct wl_shm* pointer_shared_memory;
    struct wl_surface* pointer_surface;
    struct zwp_pointer_constraints_v1* pointer_constraints;
    struct zwp_relative_pointer_manager_v1* relative_pointer_manager;
    struct zwp_relative_pointer_v1* relative_pointer;
    void* pixels;
    uint16_t width = 1920;
    uint16_t height = 1080;
    uint8_t constant_byte = 0;
    uint8_t close_xdg_toplevel;
    struct wl_display* display;
    struct wl_registry* registry;
    struct wl_callback* frame_callback;
    struct xdg_surface* xdg_surface;
};

// struct XDG_topLevelData {
// 	uint16_t width;
// 	uint16_t height;
// 	wl_shm* shared_memory;
// 	void* pixels;
// 	wl_buffer* buffer;
// 	uint8_t  close_xdg_toplevel;
// };

// struct XDG_surfaceConfigData {
// 	uint16_t width;
// 	uint16_t height;
// 	wl_shm* shared_memory;
// 	void* pixels;
// 	wl_buffer* buffer;
// 	uint8_t  constant_byte;
// 	wl_surface* wl_surface;
// };

// struct RegistryListenerData {
// 	wl_shm*        pointer_shared_memory;
// 	wl_surface*    pointer_surface;
// 	zwp_pointer_constraints_v1 *pointer_constraints;
// 	wl_surface* wl_surface;
// 	zwp_relative_pointer_manager_v1* relative_pointer_manager;
// 	zwp_relative_pointer_v1* relative_pointer;
// 	wl_pointer*    pointer;
// 	wl_keyboard*   keyboard;
// 	wl_display*  display;
// 	wl_compositor* compositor;
// 	wl_shm*        shared_memory;
// 	xdg_wm_base*   xdg_shell;
// 	wl_seat*       seat;
// };

void xdg_toplevel_configure(
    void* data,
    struct xdg_toplevel* xdg_toplevel,
    int32_t new_width,
    int32_t new_height,
    struct wl_array* atate
);
void xdg_toplevel_close(void* data, struct xdg_toplevel* xdg_toplevel);
int32_t alocate_shared_memory(uint64_t size);
void resize(void* data);
void draw(void* data);
void xdg_surface_configure(
    void* data,
    struct xdg_surface* xdg_surface,
    uint32_t serial
);
void new_frame(
    void* data,
    struct wl_callback* frame_call_back,
    uint32_t callback_data
);
void shell_ping(void* data, struct xdg_wm_base* shell, uint32_t serial);
void keyboard_keymap(
    void* data,
    struct wl_keyboard* keyboard,
    uint32_t format,
    int32_t keymap_file_descriptor,
    uint32_t size
);
void keyboard_enter(
    void* data,
    struct wl_keyboard* keyboard,
    uint32_t serial,
    struct wl_surface* surface,
    struct wl_array* keys
);
void keyboard_leave(
    void* data,
    struct wl_keyboard* keyboard,
    uint32_t serial,
    struct wl_surface* surface
);
void keyboard_key(
    void* data,
    struct wl_keyboard* keyboard,
    uint32_t serial,
    uint32_t time,
    uint32_t key,
    uint32_t state
);
void keyboard_modifiers(
    void* data,
    struct wl_keyboard* keyboard,
    uint32_t serial,
    uint32_t mods_depressed,
    uint32_t mods_latched,
    uint32_t mods_locked,
    uint32_t group
);
void keyboard_repeat_info(
    void* data,
    struct wl_keyboard* keyboard,
    int32_t rate,
    int32_t delay
);
void pointer_enter(
    void* data,
    struct wl_pointer* pointer,
    uint32_t serial,
    struct wl_surface* surface,
    wl_fixed_t sx,
    wl_fixed_t sy
);
void pointer_leave(
    void* data,
    struct wl_pointer* pointer,
    uint32_t serial,
    struct wl_surface* surface
);
void pointer_motion(
    void* data,
    struct wl_pointer* pointer,
    uint32_t time,
    wl_fixed_t sx,
    wl_fixed_t sy
);
void pointer_axis(
    void* data,
    struct wl_pointer* pointer,
    uint32_t time,
    uint32_t axis,
    wl_fixed_t value
);
void pointer_button(
    void* data,
    struct wl_pointer* pointer,
    uint32_t serial,
    uint32_t time,
    uint32_t button,
    uint32_t state
);
void handle_relative_motion(
    void* data,
    struct zwp_relative_pointer_v1* rel_pointer,
    uint32_t utime_hi,
    uint32_t utime_lo,
    wl_fixed_t dx,
    wl_fixed_t dy,
    wl_fixed_t dx_unaccel,
    wl_fixed_t dy_unaccel
);
void seat_capabilities(void* data, struct wl_seat* seat, uint32_t capabilities);
void seat_name(void* data, struct wl_seat* seat, const char* name);
void registry_global(
    void* data,
    struct wl_registry* registry,
    uint32_t name,
    const char* interface,
    uint32_t version
);
void registry_global_remove(
    void* data,
    struct wl_registry* registry,
    uint32_t name
);

[[nodiscard]] WindowWaylandVulkan* initializeWaylandWindow();
}; // namespace GLVM::core

#endif

#endif
