// This file is part of Game Loop Versatile Modules (GLVM)
// Copyright © 2024 Maksim Manokhin a.k.a. Yuriorkis_Scream. Contacts:
// <fellfrostqtw@gmail.com> Author: Maksim Manokhin a.k.a. Yuriorkis_Scream
// License: http://opensource.org/licenses/MIT

#include "glvm/Event.hpp"
#include "glvm/EventsStack.hpp"
#include "glvm/Vector.hpp"
#include "glvm/VkStructs.hpp"

extern GLVM::core::CEvent g_eEvent;
extern GLVM::core::vector<GLVM::core::MeshAxisMaxAbsoluteValues>
    allMeshMaxAbsoluteValues; /// contain all maximum absolute axis values
// extern struct wl_surface*    wl_surface;
// extern struct wl_compositor* compositor;
// extern struct xdg_toplevel*  xdg_topLevel;
// extern struct xdg_wm_base*   xdg_shell;
// extern struct wl_buffer*     buffer;
// extern struct wl_shm*        shared_memory;
// extern struct wl_seat*       seat;
// extern struct wl_keyboard*   keyboard;
// extern struct wl_pointer*    pointer;
// extern struct wl_shm*        pointer_shared_memory;
// extern struct wl_surface*    pointer_surface;
// extern struct zwp_pointer_constraints_v1 *pointer_constraints;
// extern struct zwp_relative_pointer_manager_v1* relative_pointer_manager;
// extern struct zwp_relative_pointer_v1* relative_pointer;
// extern void* pixels;
// extern uint16_t width;
// extern uint16_t height;
// extern uint8_t  constant_byte;
// extern uint8_t  close_xdg_toplevel;
// extern struct wl_display*  display;
// extern struct wl_registry* registry;
// extern struct wl_callback* frame_callback;
// extern struct xdg_surface *xdg_surface;
extern GLVM::core::CStack Input_Stack_;

extern int x_pointer;
extern int y_pointer;
extern int keys_pressed[6];

#define ARCHETYPE_CHUNK_SIZE 16384
