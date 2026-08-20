#pragma once

#include "glvm/vertex_math.hpp"

namespace glvm::ecs::components {
struct directionalLight {
    Vector<float, 3> position;
    Vector<float, 3> direction;

    Vector<float, 3> ambient;
    Vector<float, 3> diffuse;
    Vector<float, 3> specular;
};
} // namespace glvm::ecs::components
