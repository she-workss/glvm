#pragma once

#include "glvm/VertexMath.hpp"

namespace glvm::ecs::components {
struct spotLight {
    vec3 position;
    vec3 direction;
    float cutOff;
    float outerCutOff;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float constant;
    float linear;
    float quadratic;
};
} // namespace glvm::ecs::components
