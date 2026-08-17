// This file is part of Game Loop Versatile Modules (GLVM)
// Copyright © 2024 Maksim Manokhin a.k.a. Yuriorkis_Scream. Contacts:
// <fellfrostqtw@gmail.com> Author: Maksim Manokhin a.k.a. Yuriorkis_Scream
// License: http://opensource.org/licenses/MIT

#ifndef COLLIDER_COMPONENT_HPP
#define COLLIDER_COMPONENT_HPP

#include "glvm/Vector.hpp"

#include <vector>

namespace GLVM::ecs::components {
class collider {
public:
    core::vector<unsigned int> colliders;
};
} // namespace GLVM::ecs::components

#endif
