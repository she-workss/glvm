#pragma once

#include "glvm/Texture.hpp"
#include "glvm/VertexMath.hpp"

namespace glvm::ecs::components {
struct material {
    ecs::TextureHandle diffuseTextureID_ = {};
    ecs::TextureHandle specularTextureID_ = {};
    vec3 ambient = {0.0f, 0.0f, 0.0f};
    float shininess = 0.0f;
};
} // namespace glvm::ecs::components
