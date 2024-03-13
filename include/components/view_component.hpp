// This file is part of Game Loop Versatile Modules (GLVM)
// Copyright © 2024 Maksim Manokhin a.k.a. Yuriorkis_Scream. Contacts:
// <fellfrostqtw@gmail.com> Author: Maksim Manokhin a.k.a. Yuriorkis_Scream
// License: http://opensource.org/licenses/MIT
#pragma once

#include "vertex_math.hpp"

namespace GLVM::ecs::components {
class beholder {
public:
    Vector<float, 3> forward {0.0f, 0.0, 0.0f};
    Vector<float, 3> up {0.0f, 0.0f, 0.0f};
    Vector<float, 3> right {0.0f, 0.0f, 0.0f};
    Vector<float, 3> Position {0.0f, 0.0f, 0.0f};
};
} // namespace GLVM::ecs::components
