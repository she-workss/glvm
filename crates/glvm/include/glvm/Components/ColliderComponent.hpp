#pragma once

#include "glvm/Vector.hpp"

#include <vector>

namespace glvm::ecs::components {
class collider {
public:
    core::vector<unsigned int> colliders;
};
} // namespace glvm::ecs::components
