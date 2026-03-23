// This file is part of Game Loop Versatile Modules (GLVM)
// Copyright © 2024 Maksim Manokhin a.k.a. Yuriorkis_Scream. Contacts:
// <fellfrostqtw@gmail.com> Author: Maksim Manokhin a.k.a. Yuriorkis_Scream
// License: http://opensource.org/licenses/MIT
#pragma once

#include "glvm/texture.hpp"
#include "glvm/vertex_math.hpp"

namespace GLVM::ecs::components {
struct material {
    ecs::TextureHandle diffuseTextureID_;
    ecs::TextureHandle specularTextureID_;
    // This field using to choose specific instance of texture image in Vulkan.
    unsigned int vkInnerId_;
    vec3 ambient;
    float shininess;
};
} // namespace GLVM::ecs::components
