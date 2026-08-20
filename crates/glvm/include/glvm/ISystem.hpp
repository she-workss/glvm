#pragma once

#include "glvm/ComponentManager.hpp"
#include "glvm/Event.hpp"

namespace glvm::ecs {
class ISystem {
public:
    virtual ~ISystem() {}

    virtual void Update() = 0;
};
} // namespace glvm::ecs
