// This file is part of Game Loop Versatile Modules (GLVM)
// Copyright © 2024 Maksim Manokhin a.k.a. Yuriorkis_Scream. Contacts:
// <fellfrostqtw@gmail.com> Author: Maksim Manokhin a.k.a. Yuriorkis_Scream
// License: http://opensource.org/licenses/MIT
#pragma once

namespace GLVM::ecs::components {
class collider {
public:
    bool bGround_Collision_ = false;
    bool roofCollision = false;
    bool bWall_Collision_ = false;
};
} // namespace GLVM::ecs::components
