#pragma once

#include "glvm/Vector.hpp"

namespace glvm::ecs::components {
struct ItemSlotType {
    unsigned int height;
    unsigned int width;
};

struct item {
    // Array that contain entities with inventorySlotComponent.
    core::vector<unsigned int> occupiedSlots;
    ItemSlotType itemSlotType;
    bool isActor;
};
} // namespace glvm::ecs::components
