#pragma once

#include "glvm/Vector.hpp"

namespace glvm::ecs::components {
struct font {
    core::vector<char> font_string;
    float lifeTime;
    bool removeble;
};
} // namespace glvm::ecs::components
