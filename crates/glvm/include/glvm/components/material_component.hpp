#pragma once

#include "glvm/texture.hpp"
#include "glvm/vertex_math.hpp"

namespace glvm::ecs::components {
struct material {
    ecs::TextureHandle diffuseTextureID_ = {};
    ecs::TextureHandle specularTextureID_ = {};
    Vector<float, 3> ambient = {0.0f, 0.0f, 0.0f};
    float shininess = 0.0f;
};
} // namespace glvm::ecs::components
