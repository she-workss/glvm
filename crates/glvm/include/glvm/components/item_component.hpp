#pragma once

#include <vector>

namespace glvm::ecs::components {
struct ItemSlotType {
    unsigned int height;
    unsigned int width;
};

struct item {
    // Array that contain entities with inventorySlotComponent.
    std::vector<unsigned int> occupiedSlots;
    ItemSlotType itemSlotType;
    bool isActor;
};
} // namespace glvm::ecs::components
