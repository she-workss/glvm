#pragma once

#include <vector>

namespace glvm::ecs::components {
struct font {
    std::vector<char> font_string;
    float lifeTime;
    bool removeble;
};
} // namespace glvm::ecs::components
