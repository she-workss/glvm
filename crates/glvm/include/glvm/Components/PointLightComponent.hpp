#pragma once

#include "glvm/VertexMath.hpp"

namespace glvm::ecs::components {
struct pointLight {
    vec3 position;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float constant;
    float linear;
    float quadratic;
};
} // namespace glvm::ecs::components
