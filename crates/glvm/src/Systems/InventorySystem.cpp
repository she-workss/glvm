#include "glvm/Systems/InventorySystem.hpp"

#include "glvm/ArchetypeECS/ArchECS_Types.hpp"
#include "glvm/ArchetypeECS/ArchECS_World.hpp"
#include "glvm/Archetypes/CrosshairArchetype.hpp"
#include "glvm/Archetypes/EnemyArchetype.hpp"
#include "glvm/Archetypes/InventoryArchetype.hpp"
#include "glvm/Archetypes/ItemArchetype.hpp"
#include "glvm/Archetypes/PlayerArchetype.hpp"
#include "glvm/Components/CrosshairComponent.hpp"
#include "glvm/Components/InventoryComponent.hpp"
#include "glvm/Components/ItemComponent.hpp"
#include "glvm/Components/TransformComponent.hpp"
#include "glvm/TagComponents/CrosshairTagComponent.hpp"
#include "glvm/VertexMath.hpp"

#include <cstdint>

namespace glvm::ecs {
void InventorySystem::Update() {
    if (isInventoryOpened) {
        namespace cm = glvm::ecs::components;

        crosshairArchetypesNumber = 0;
        arch::world.searchCacheArchetypes(
            crosshairRequiredMask,
            &archView.crosshairCachedArchetype,
            crosshairArchetypesNumber
        );
        componentsView.crosshairTransformsView =
            (ecs::components::transform*)archView.crosshairCachedArchetype
                ->components[arch::ComponentsIndices::TRANSFORM_COMPONENT];

        inventoryArchetypesNumber = 0;
        arch::world.searchCacheArchetypes(
            inventoryRequiredMask,
            &archView.inventoryCachedArchetype,
            inventoryArchetypesNumber
        );

        componentsView.inventoryTransformsView =
            (ecs::components::transform*)archView.inventoryCachedArchetype
                ->components[arch::ComponentsIndices::TRANSFORM_COMPONENT];
        componentsView.inventoryView =
            (ecs::components::inventory*)archView.inventoryCachedArchetype
                ->components[arch::ComponentsIndices::INVENTORY_COMPONENT];
        componentsView.inventoryMeshesView =
            (ecs::components::mesh*)archView.inventoryCachedArchetype
                ->components[arch::ComponentsIndices::MESH_COMPONENT];

        if (componentsView.crosshairTransformsView
            && componentsView.inventoryTransformsView
            && componentsView.inventoryView
            && componentsView.inventoryMeshesView) {
            cm::transform* crosshairTransformComponent =
                &componentsView.crosshairTransformsView[0];

            cm::transform* inventoryTransformComponent =
                &componentsView.inventoryTransformsView[0];
            cm::inventory* inventoryComponent =
                &componentsView.inventoryView[0];
            cm::mesh* inventoryMeshComponent =
                &componentsView.inventoryMeshesView[0];

            const float inventorySlotScale = inventoryMeshComponent->gltf
                ? inventoryComponent->slotScale * 2.0f
                : inventoryComponent->slotScale;
            const float inventorySlotHalfScale = inventoryMeshComponent->gltf
                ? inventoryComponent->slotScale
                : inventoryComponent->slotScale * 0.5f;

            // Take an item from inventory.
            if (*isItemDraged < 0 && isLeftMouseButtonPressed
                && *isLeftMouseButtonReleased) {
                if (checkCrosshairInventoryIntersection(
                        crosshairTransformComponent,
                        inventoryTransformComponent,
                        inventoryComponent,
                        inventorySlotScale,
                        inventorySlotHalfScale
                    )) {
                    point2D<int> intersectionSlot =
                        determineActualIntersectionSlot(
                            crosshairTransformComponent,
                            inventoryTransformComponent,
                            inventorySlotScale,
                            inventorySlotHalfScale
                        );
                    const unsigned int row = intersectionSlot.y;
                    const unsigned int column = intersectionSlot.x;
                    const unsigned int entity =
                        inventoryComponent->slots[row][column];
                    // Check slot is not empty and hold an item.
                    if (entity != UINT_MAX && entity >= 0) {
                        arch::EntityLocation itemLocation =
                            arch::world.entityLocations[arch::getId(entity)];
                        arch::ItemArchetype* itemArch =
                            static_cast<arch::ItemArchetype*>(itemLocation.arch);
                        const uint32_t itemIndex = itemLocation.index;
                        cm::item* itemComponent = &itemArch->items[itemIndex];

                        if (itemComponent != nullptr) {
                            for (unsigned int i = 0;
                                 i < itemComponent->occupiedSlots.GetSize();
                                 ++i) {
                                unsigned int row_index =
                                    itemComponent->occupiedSlots[i]
                                    / inventoryComponent->row;
                                unsigned int col_index =
                                    itemComponent->occupiedSlots[i]
                                    % inventoryComponent->col;
                                // Need to free all slots that hold an item.
                                inventoryComponent->slots[row_index][col_index] =
                                    UINT_MAX;
                            }
                        }
                        // Set currently dragged item entity.
                        *isItemDraged = entity;
                    }
                }
                *isLeftMouseButtonReleased = false;
            }

            if (*isItemDraged >= 0) {
                if (checkCrosshairInventoryIntersection(
                        crosshairTransformComponent,
                        inventoryTransformComponent,
                        inventoryComponent,
                        inventorySlotScale,
                        inventorySlotHalfScale
                    )) {
                    [[maybe_unused]] point2D<int> intersectionSlot =
                        determineActualIntersectionSlot(
                            crosshairTransformComponent,
                            inventoryTransformComponent,
                            inventorySlotScale,
                            inventorySlotHalfScale
                        );

                    arch::EntityLocation itemLocation =
                        arch::world.entityLocations[arch::getId(*isItemDraged)];
                    arch::ItemArchetype* itemArch =
                        static_cast<arch::ItemArchetype*>(itemLocation.arch);
                    const uint32_t itemIndex = itemLocation.index;
                    cm::item* itemComponent = &itemArch->items[itemIndex];

                    core::vector<unsigned int> potentialOccupiedSlots;
                    int isSwapable = 0;
                    isSwapable = determineSwappableStatusAndSlots(
                        itemComponent,
                        inventoryTransformComponent,
                        potentialOccupiedSlots,
                        crosshairTransformComponent,
                        intersectionSlot,
                        inventoryComponent,
                        inventorySlotScale
                    );

                    inventoryComponent->highlightedSlots =
                        potentialOccupiedSlots;
                    if (isSwapable == -1 || isSwapable >= 0) {
                        inventoryComponent->isAvailableHighlightedSlots = true;
                    } else {
                        inventoryComponent->isAvailableHighlightedSlots = false;
                    }
                } else {
                    inventoryComponent->highlightedSlots.clear();
                }
            }

            // Item drop to inventory, swapped or we just can't place.
            if (*isItemDraged >= 0 && isLeftMouseButtonPressed
                && *isLeftMouseButtonReleased) {
                int isSwapable = 0;
                if (checkCrosshairInventoryIntersection(
                        crosshairTransformComponent,
                        inventoryTransformComponent,
                        inventoryComponent,
                        inventorySlotScale,
                        inventorySlotHalfScale
                    )) {
                    [[maybe_unused]] point2D<int> intersectionSlot =
                        determineActualIntersectionSlot(
                            crosshairTransformComponent,
                            inventoryTransformComponent,
                            inventorySlotScale,
                            inventorySlotHalfScale
                        );

                    arch::EntityLocation itemLocation =
                        arch::world.entityLocations[arch::getId(*isItemDraged)];
                    arch::ItemArchetype* itemArch =
                        static_cast<arch::ItemArchetype*>(itemLocation.arch);
                    const uint32_t itemIndex = itemLocation.index;
                    cm::item* itemComponent = &itemArch->items[itemIndex];

                    core::vector<unsigned int> potentialOccupiedSlots;
                    isSwapable = determineSwappableStatusAndSlots(
                        itemComponent,
                        inventoryTransformComponent,
                        potentialOccupiedSlots,
                        crosshairTransformComponent,
                        intersectionSlot,
                        inventoryComponent,
                        inventorySlotScale
                    );

                    const int itemWidth = itemComponent->itemSlotType.width;
                    const int itemHeight = itemComponent->itemSlotType.height;

                    // Default value. Just drop item to all empty slots.
                    if (isSwapable == -1) {
                        itemComponent->occupiedSlots = potentialOccupiedSlots;
                        fillInventorySlots(
                            itemComponent,
                            itemWidth,
                            itemHeight,
                            inventoryComponent,
                            *isItemDraged
                        );
                        // Swap one item that we dragging to another one in
                        // inventory.
                    } else if (isSwapable > 0) {
                        arch::EntityLocation itemLocation =
                            arch::world.entityLocations[arch::getId(isSwapable)];
                        arch::ItemArchetype* itemArch =
                            static_cast<arch::ItemArchetype*>(itemLocation.arch);
                        const uint32_t itemIndex = itemLocation.index;
                        cm::item* swapedItemComponent =
                            &itemArch->items[itemIndex];

                        itemComponent->occupiedSlots = potentialOccupiedSlots;
                        fillInventorySlots(
                            swapedItemComponent,
                            swapedItemComponent->itemSlotType.width,
                            swapedItemComponent->itemSlotType.height,
                            inventoryComponent,
                            UINT_MAX
                        );

                        swapedItemComponent->occupiedSlots.clear();
                        fillInventorySlots(
                            itemComponent,
                            itemWidth,
                            itemHeight,
                            inventoryComponent,
                            *isItemDraged
                        );
                    }

                    if (isSwapable == -1) {
                        *isLeftMouseButtonReleased = false;
                        *isItemDraged = -1;
                        // Already have 2 or more items in potential inventory
                        // slots.
                    } else if (isSwapable == -2) {
                        *isLeftMouseButtonReleased = false;
                    } else {
                        *isLeftMouseButtonReleased = false;
                        *isItemDraged = isSwapable;
                    }
                    // Item drop to the ground.
                } else {
                    arch::EntityLocation itemLocation =
                        arch::world.entityLocations[arch::getId(*isItemDraged)];
                    arch::ItemArchetype* itemArch =
                        static_cast<arch::ItemArchetype*>(itemLocation.arch);
                    const uint32_t itemIndex = itemLocation.index;
                    itemArch->rigidBodies[itemIndex] = {.fMass_ = 2.0f};
                    cm::transform* itemTransform =
                        &itemArch->transforms[itemIndex];
                    cm::item* item = &itemArch->items[itemIndex];
                    item->isActor = true;
                    // Remove this cringe.
                    const uint32_t player = 0;
                    arch::EntityLocation playerLocation =
                        arch::world.entityLocations[arch::getId(player)];
                    arch::PlayerArchetype* playerArch =
                        static_cast<arch::PlayerArchetype*>(playerLocation.arch);
                    const uint32_t playerIndex = playerLocation.index;
                    cm::transform* playerTransform =
                        &playerArch->transforms[playerIndex];
                    itemTransform->position = playerTransform->position;
                    vec3 normalizedForward =
                        Normalize(playerTransform->forward);
                    itemTransform->position[0] += normalizedForward[0] * 2.5f;
                    itemTransform->position[1] += normalizedForward[1] * 2.5f;
                    itemTransform->position[2] += normalizedForward[2] * 2.5f;
                    itemTransform->scale = 0.05f;

                    *isItemDraged = -1;
                    *isLeftMouseButtonReleased = false;
                }
            }
        }
    }
}

int InventorySystem::determineSwappableStatusAndSlots(
    components::item* itemComponent,
    components::transform* inventoryTransformComponent,
    core::vector<unsigned int>& potentialOccupiedSlots,
    components::transform* crosshairTransformComponent,
    point2D<int> intersectionSlot,
    components::inventory* inventoryComponent,
    const float inventorySlotScale
) {
    if (itemComponent != nullptr) {
        const int itemWidth = itemComponent->itemSlotType.width;
        const int itemHeight = itemComponent->itemSlotType.height;

        const int row = intersectionSlot.y;
        const int column = intersectionSlot.x;

        // Find left-upper pivot slot inventory.
        int rowBasicOffset = 0;
        int columnBasicOffset = 0;

        // Set as pivot point slot in left upper corner.
        // Need to calculate offset for row and column
        // to change it from center. And need to It is
        // necessary to take into account the offset
        // relative to the center for additional correction.
        columnBasicOffset = calculateBasicOffset(
            itemWidth,
            inventoryTransformComponent->position[0],
            crosshairTransformComponent->position[0],
            column,
            inventorySlotScale
        );
        rowBasicOffset = calculateBasicOffset(
            itemHeight,
            inventoryTransformComponent->position[1],
            crosshairTransformComponent->position[1],
            row,
            inventorySlotScale * aspectRate
        );

        int pivotRow = row - rowBasicOffset;
        int pivotColumn = column - columnBasicOffset;

        clamp<int>(
            0,
            pivotRow,
            static_cast<int>(inventoryComponent->row) - itemHeight
        );
        clamp<int>(
            0,
            pivotColumn,
            static_cast<int>(inventoryComponent->col) - itemWidth
        );

        return determineSwappableField(
            itemComponent,
            itemWidth,
            itemHeight,
            pivotRow,
            pivotColumn,
            inventoryComponent,
            potentialOccupiedSlots
        );
    } else {
        // Return -3 as error code means itemComponent is nullptr.
        return -3;
    }
}

void InventorySystem::fillInventorySlots(
    components::item* itemComponent,
    const int itemWidth,
    const int itemHeight,
    components::inventory* inventoryComponent,
    const int fillValue
) {
    for (int i = 0; i < itemHeight; ++i) {
        for (int j = 0; j < itemWidth; ++j) {
            const unsigned int slotsRow =
                itemComponent->occupiedSlots[i * itemWidth + j]
                / inventoryComponent->col;
            const unsigned int slotsColumn =
                itemComponent->occupiedSlots[i * itemWidth + j]
                % inventoryComponent->col;

            inventoryComponent->slots[slotsRow][slotsColumn] = fillValue;
        }
    }
}

int InventorySystem::determineSwappableField(
    components::item* itemComponent,
    const int itemWidth,
    const int itemHeight,
    int pivotRow,
    int pivotColumn,
    components::inventory* inventoryComponent,
    core::vector<unsigned int>& potentialOccupiedSlots
) {
    itemComponent->occupiedSlots.clear();
    // -1: default value. -2: found two entities in potential slots. Any other
    // value: swappable.
    int isSwapable = -1;
    for (int i = 0; i < itemHeight; ++i) {
        for (int j = 0; j < itemWidth; ++j) {
            const unsigned int finalRow = pivotRow + i;
            const unsigned int finalColumn = pivotColumn + j;
            if (isSwapable == -1
                && inventoryComponent->slots[finalRow][finalColumn]
                    != UINT_MAX) {
                isSwapable = inventoryComponent->slots[finalRow][finalColumn];
            } else if (
                isSwapable > 0
                && inventoryComponent->slots[finalRow][finalColumn] != UINT_MAX
                && (int)inventoryComponent->slots[finalRow][finalColumn]
                    != isSwapable
            ) {
                isSwapable = -2;
            }

            potentialOccupiedSlots.Push(
                finalRow * inventoryComponent->col + finalColumn
            );
        }
    }

    return isSwapable;
}

int InventorySystem::calculateBasicOffset(
    const int itemAxisSize,
    const float axisValue,
    const float crosshairAxisPosition,
    const int axisSlotIndex,
    const float inventorySlotScale
) {
    if (itemAxisSize % 2 == 0) {
        const float slotCenterX =
            axisValue + static_cast<float>(axisSlotIndex) * inventorySlotScale;
        if (slotCenterX > crosshairAxisPosition) {
            return itemAxisSize / 2;
        }
        return itemAxisSize / 2 - 1;
    }
    return itemAxisSize / 2;
}

bool InventorySystem::checkCrosshairInventoryIntersection(
    components::transform* crosshairTransformComponent,
    components::transform* inventoryTransformComponent,
    components::inventory* inventoryComponent,
    const float inventorySlotScale,
    const float inventorySlotHalfScale
) {
    return crosshairTransformComponent->position[0]
        > inventoryTransformComponent->position[0] - inventorySlotHalfScale
        && crosshairTransformComponent->position[0]
        < inventoryTransformComponent->position[0] - inventorySlotHalfScale
            + inventorySlotScale * inventoryComponent->col
        && crosshairTransformComponent->position[1]
        > inventoryTransformComponent->position[1]
            - inventorySlotHalfScale * aspectRate
        && crosshairTransformComponent->position[1]
        < inventoryTransformComponent->position[1]
            - inventorySlotHalfScale * aspectRate
            + inventorySlotScale * inventoryComponent->row * aspectRate;
}

point2D<int> InventorySystem::determineActualIntersectionSlot(
    components::transform* crosshairTransformComponent,
    components::transform* inventoryTransformComponent,
    const float inventorySlotScale,
    const float inventorySlotHalfScale
) {
    float x_delta = crosshairTransformComponent->position[0]
        - inventoryTransformComponent->position[0] + inventorySlotHalfScale;
    float y_delta = crosshairTransformComponent->position[1]
        - inventoryTransformComponent->position[1]
        + inventorySlotHalfScale * aspectRate;

    return point2D<int> {
        (int)(x_delta / inventorySlotScale),
        (int)(y_delta / (inventorySlotScale * aspectRate))
    };
}
} // namespace glvm::ecs
