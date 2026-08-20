#pragma once

#include "glvm/VertexMath.hpp"

namespace glvm::ecs::components {
struct directionalLight {
    vec3 position;
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};
} // namespace glvm::ecs::components
