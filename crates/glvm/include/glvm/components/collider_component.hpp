#pragma once

#include <vector>

namespace glvm::ecs::components {
class collider {
public:
    std::vector<unsigned int> colliders;
};
} // namespace glvm::ecs::components
