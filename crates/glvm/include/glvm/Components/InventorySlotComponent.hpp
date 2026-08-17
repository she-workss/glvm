#ifndef INVENTORY_SLOT_COMPONENT
#define INVENTORY_SLOT_COMPONENT

#include <climits>

namespace GLVM::ecs::components {
struct inventorySlot {
    unsigned int itemEntity = UINT_MAX;
};
} // namespace GLVM::ecs::components

#endif
