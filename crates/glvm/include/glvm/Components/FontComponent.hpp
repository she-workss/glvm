#ifndef FONT_COMPONENT_HPP
#define FONT_COMPONENT_HPP

#include "glvm/Vector.hpp"

namespace GLVM::ecs::components {
struct font {
    core::vector<char> font_string;
    float lifeTime;
    bool removeble;
};
} // namespace GLVM::ecs::components

#endif
