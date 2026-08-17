// This file is part of Game Loop Versatile Modules (GLVM)
// Copyright © 2024 Maksim Manokhin a.k.a. Yuriorkis_Scream. Contacts:
// <fellfrostqtw@gmail.com> Author: Maksim Manokhin a.k.a. Yuriorkis_Scream
// License: http://opensource.org/licenses/MIT

#ifndef RIGIDBODY_COMPONENT_HPP
#define RIGIDBODY_COMPONENT_HPP

#include "glvm/VertexMath.hpp"

namespace GLVM::ecs::components {
class rigidBody {
public:
    float fMass_ = 0.0f;
    float jumpAccumulator = 0.0f;
};
} // namespace GLVM::ecs::components

#endif
