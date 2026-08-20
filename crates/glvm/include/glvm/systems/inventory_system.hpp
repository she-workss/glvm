#pragma once

#include "glvm/archetype_ecs/arch_ecs_types.hpp"
#include "glvm/archetype_ecs/arch_ecs_utils.hpp"
#include "glvm/archetype_ecs/archetype_interface.hpp"
#include "glvm/components/crosshair_component.hpp"
#include "glvm/components/inventory_component.hpp"
#include "glvm/components/item_component.hpp"
#include "glvm/components/transform_component.hpp"
#include "glvm/i_system.hpp"
#include "glvm/vertex_math.hpp"

#include <print>

namespace glvm::ecs {
class InventorySystem: public ecs::ISystem {
public:
    uint32_t crosshairArchetypesNumber = 0;
    uint32_t inventoryArchetypesNumber = 0;

    struct ArchView {
        arch::Archetype* crosshairCachedArchetype = nullptr;
        arch::Archetype* inventoryCachedArchetype = nullptr;
    } archView;

    struct ComponentsView {
        ecs::components::transform* crosshairTransformsView = nullptr;

        ecs::components::transform* inventoryTransformsView = nullptr;
        ecs::components::inventory* inventoryView = nullptr;
        ecs::components::mesh* inventoryMeshesView = nullptr;
    } componentsView;

    uint64_t crosshairRequiredMask =
        (1ull << ecs::arch::ComponentsIndices::TRANSFORM_COMPONENT)
        | (1ull << ecs::arch::ComponentsIndices::CROSSHAIR_TAG_COMPONENT);

    uint64_t inventoryRequiredMask =
        (1ull << ecs::arch::ComponentsIndices::TRANSFORM_COMPONENT)
        | (1ull << ecs::arch::ComponentsIndices::INVENTORY_COMPONENT)
        | (1ull << ecs::arch::ComponentsIndices::MESH_COMPONENT);

    void Update() override;
    int determineSwappableStatusAndSlots(
        components::item* itemComponent,
        components::transform* inventoryTransformComponent,
        std::vector<unsigned int>& potentialOccupiedSlots,
        components::transform* crosshairTransformComponent,
        point2D<int> intersectionSlot,
        components::inventory* inventoryComponent,
        const float inventorySlotScale
    );
    void fillInventorySlots(
        components::item* itemComponent,
        const int itemWidth,
        const int itemHeight,
        components::inventory* inventoryComponent,
        const int fillValue
    );
    int determineSwappableField(
        components::item* itemComponent,
        const int itemWidth,
        const int itemHeight,
        int pivotRow,
        int pivotColumn,
        components::inventory* inventoryComponent,
        std::vector<unsigned int>& potentialOccupiedSlots
    );
    int calculateBasicOffset(
        const int itemAxisSize,
        const float axisValue,
        const float crosshairAxisPosition,
        const int axisSlotIndex,
        const float inventorySlotScale
    );
    bool checkCrosshairInventoryIntersection(
        components::transform* crosshairTransformComponent,
        components::transform* inventoryTransformComponent,
        components::inventory* inventoryComponent,
        const float inventorySlotScale,
        const float inventorySlotHalfScale
    );
    point2D<int> determineActualIntersectionSlot(
        components::transform* crosshairTransformComponent,
        components::transform* inventoryTransformComponent,
        const float inventorySlotScale,
        const float inventorySlotHalfScale
    );

    bool isInventoryOpened;
    int* isItemDraged;
    bool* isLeftMouseButtonReleased;
    bool isLeftMouseButtonPressed;
    float mouseOffsetX = 0;
    float mouseOffsetY = 0;
    // Window aspect ratio, set by engine each frame.
    float aspectRate = 0.0f;
    arch::Archetype* cachedCrosshairArchetype;
    arch::Archetype* cachedInventoryArchetype;
};
} // namespace glvm::ecs
