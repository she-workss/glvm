// This file is part of Game Loop Versatile Modules (GLVM)
// Copyright © 2024 Maksim Manokhin a.k.a. Yuriorkis_Scream. Contacts:
// <fellfrostqtw@gmail.com> Author: Maksim Manokhin a.k.a. Yuriorkis_Scream
// License: http://opensource.org/licenses/MIT

#ifndef VIEW_COMPONENT_HPP
#define VIEW_COMPONENT_HPP

#include "glvm/VertexMath.hpp"

namespace GLVM::ecs::components {
class beholder {
public:
    vec3 Position {0.0f, 0.0f, 0.0f};
    vec3 forward {0.0f, 0.0, 0.0f};
};
} // namespace GLVM::ecs::components

#endif
