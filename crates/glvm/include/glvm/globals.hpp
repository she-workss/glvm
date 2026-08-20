#pragma once

#include "glvm/event.hpp"
#include "glvm/events_stack.hpp"
#include "glvm/vk_structs.hpp"

#include <vector>

extern glvm::core::CEvent g_eEvent;

// Contains all maximum absolute axis values.
extern std::vector<glvm::core::MeshAxisMaxAbsoluteValues>
    allMeshMaxAbsoluteValues;

extern glvm::core::CStack Input_Stack_;

extern int x_pointer;
extern int y_pointer;
extern int keys_pressed[6];

#define ARCHETYPE_CHUNK_SIZE 16384
