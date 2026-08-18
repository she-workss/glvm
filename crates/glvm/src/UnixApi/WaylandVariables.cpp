#include "glvm/EventsStack.hpp"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

// struct wl_surface*    wl_surface;
// struct wl_compositor* compositor;
// struct xdg_toplevel*  xdg_topLevel;
// struct xdg_wm_base*   xdg_shell;
// struct wl_buffer*     buffer;
// struct wl_shm*        shared_memory;
// struct wl_seat*       seat;
// struct wl_keyboard*   keyboard;
// struct wl_pointer*    pointer;
// struct wl_shm*        pointer_shared_memory;
// struct wl_surface*    pointer_surface;
// struct zwp_pointer_constraints_v1 *pointer_constraints;
// struct zwp_relative_pointer_manager_v1* relative_pointer_manager = NULL;
// struct zwp_relative_pointer_v1* relative_pointer = NULL;
// void* pixels;
// uint8_t  constant_byte = 0;
// uint8_t  close_xdg_toplevel;
// struct wl_display*  display;
// struct wl_registry* registry;
// struct wl_callback* frame_callback;
// struct xdg_surface* xdg_surface;
GLVM::core::CStack Input_Stack_ {};

int x_pointer;
int y_pointer;
