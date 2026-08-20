#pragma once

#include "glvm/components/inventory_slot_component.hpp"
#include "glvm/components/vertex_component.hpp"

#include <climits>
#include <vector>

namespace glvm::ecs::components {
class inventory {
public:
    inventory() {
        for (unsigned int i = 0; i < row; ++i) {
            slots[i] = new unsigned int[col];
        }

        for (unsigned int i = 0; i < row; ++i) {
            for (unsigned int j = 0; j < col; ++j) {
                slots[i][j] = -1;
            }
        }
    }

    inventory(const inventory& inv) {
        for (unsigned int i = 0; i < row; ++i) {
            this->slots[i] = new unsigned int[col];
        }

        for (unsigned int i = 0; i < row; ++i) {
            for (unsigned int j = 0; j < col; ++j) {
                this->slots[i][j] = inv.slots[i][j];
            }
        }

        this->entityOwner = inv.entityOwner;
        this->highlightedSlots = inv.highlightedSlots;
        this->isAvailableHighlightedSlots = inv.isAvailableHighlightedSlots;
    }

    ~inventory() {
        for (unsigned int i = 0; i < row; ++i) {
            delete[] slots[i];
        }

        delete[] slots;
    }

    unsigned int row = 8;
    unsigned int col = 8;
    // Array with entities contained inventorySlotComponents.
    unsigned int** slots = new unsigned int*[row];
    unsigned int entityOwner = UINT_MAX;
    std::vector<unsigned int> highlightedSlots;
    bool isAvailableHighlightedSlots = false;
    MeshHandle slotMeshID;
    float slotScale;
};
}; // namespace glvm::ecs::components
