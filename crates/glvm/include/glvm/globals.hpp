#pragma once

#include "glvm/Event.hpp"
#include "glvm/EventsStack.hpp"
#include "glvm/Vector.hpp"
#include "glvm/VkStructs.hpp"

extern glvm::core::CEvent g_eEvent;

// Contains all maximum absolute axis values.
extern glvm::core::vector<glvm::core::MeshAxisMaxAbsoluteValues>
    allMeshMaxAbsoluteValues;

extern glvm::core::CStack Input_Stack_;

extern int x_pointer;
extern int y_pointer;
extern int keys_pressed[6];

#define ARCHETYPE_CHUNK_SIZE 16384
