#pragma once

#include "glvm/archetype_ecs/arch_ecs_utils.hpp"
#include "glvm/archetype_ecs/arch_ecs_world.hpp"
#include "glvm/archetype_ecs/archetype_interface.hpp"
#include "glvm/archetypes/crosshair_archetype.hpp"
#include "glvm/archetypes/inventory_archetype.hpp"
#include "glvm/archetypes/item_archetype.hpp"
#include "glvm/component_manager.hpp"
#include "glvm/components/actor_component.hpp"
#include "glvm/components/collider_component.hpp"
#include "glvm/components/crosshair_component.hpp"
#include "glvm/components/inventory_component.hpp"
#include "glvm/components/item_component.hpp"
#include "glvm/components/rigid_body_component.hpp"
#include "glvm/components/transform_component.hpp"
#include "glvm/entity_manager.hpp"
#include "glvm/events_stack.hpp"
#include "glvm/i_system.hpp"

#include <climits>

namespace glvm::ecs {
class ItemSystem: public ISystem {
public:
    uint32_t inventoryArchetypesNumber = 0;
    uint32_t itemArchetypesNumber = 0;
    uint32_t crosshairArchetypesNumber = 0;

    struct ArchView {
        arch::Archetype* inventoryCachedArchetype = nullptr;
        arch::Archetype* itemArchetype = nullptr;
        arch::Archetype* crosshairArchetype = nullptr;
    } archView;

    struct ComponentsView {
        ecs::components::inventory* inventoriesView = nullptr;

        ecs::components::item* itemsView = nullptr;
        ecs::components::collider* itemCollidersView = nullptr;
        ecs::components::transform* itemTransformsView = nullptr;

        ecs::components::transform* crosshairTransforms = nullptr;
    } componentsView;

    uint64_t inventoryRequiredMask =
        (1ull << ecs::arch::ComponentsIndices::INVENTORY_COMPONENT);

    uint64_t itemRequiredMask =
        (1ul << arch::ComponentsIndices::TRANSFORM_COMPONENT)
        | (1ul << arch::ComponentsIndices::ITEM_COMPONENT)
        | (1ul << arch::ComponentsIndices::MESH_COMPONENT)
        | (1ul << arch::ComponentsIndices::MATERIAL_COMPONENT)
        | (1ul << arch::ComponentsIndices::COLLIDER_COMPONENT)
        | (1ul << arch::ComponentsIndices::COLLIDER_FLAGS_COMPONENT);

    uint64_t crosshairRequiredMask =
        (1ul << arch::ComponentsIndices::TRANSFORM_COMPONENT)
        | (1ul << arch::ComponentsIndices::CROSSHAIR_TAG_COMPONENT);

    void Update();
    bool putItem2x2(
        components::inventory* inventoryComponent,
        unsigned int itemEntity
    );

    core::CStack* inputStack;
    bool isInventoryOpened;
    int* dragedItemEntity;
    bool* isLeftMouseButtonReleased;
    bool isLeftMouseButtonPressed;
    float mouseOffsetX = 0;
    float mouseOffsetY = 0;
};
} // namespace glvm::ecs
