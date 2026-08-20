#pragma once
#include <climits>

namespace glvm::ecs::components {
struct inventorySlot {
    unsigned int itemEntity = UINT_MAX;
};
} // namespace glvm::ecs::components
