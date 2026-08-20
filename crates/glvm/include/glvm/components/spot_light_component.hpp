#pragma once

#include "glvm/vertex_math.hpp"

namespace glvm::ecs::components {
struct spotLight {
    Vector<float, 3> position;
    Vector<float, 3> direction;
    float cutOff;
    float outerCutOff;

    Vector<float, 3> ambient;
    Vector<float, 3> diffuse;
    Vector<float, 3> specular;

    float constant;
    float linear;
    float quadratic;
};
} // namespace glvm::ecs::components
