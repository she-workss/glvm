#pragma once

#include "glvm/component_manager.hpp"
#include "glvm/event.hpp"

namespace glvm::ecs {
class ISystem {
public:
    virtual ~ISystem() {}

    virtual void Update() = 0;
};
} // namespace glvm::ecs
