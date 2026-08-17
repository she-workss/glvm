#ifndef ITEM_COMPONENT_HPP
#define ITEM_COMPONENT_HPP

#include "glvm/Vector.hpp"

namespace GLVM::ecs::components {
struct ItemSlotType {
    unsigned int height;
    unsigned int width;
};

struct item {
    core::vector<unsigned int> occupiedSlots; ///< Array that contain entities
                                              ///< with inventorySlotComponent
    ItemSlotType itemSlotType;
    bool isActor;
};
} // namespace GLVM::ecs::components

#endif
