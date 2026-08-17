#ifndef ITEM_SYSTEM
#define ITEM_SYSTEM

#include "glvm/ArchetypeECS/ArchECS_Utils.hpp"
#include "glvm/ArchetypeECS/ArchECS_World.hpp"
#include "glvm/ArchetypeECS/ArchetypeInterface.hpp"
#include "glvm/Archetypes/CrosshairArchetype.hpp"
#include "glvm/Archetypes/InventoryArchetype.hpp"
#include "glvm/Archetypes/ItemArchetype.hpp"
#include "glvm/ComponentManager.hpp"
#include "glvm/Components/ActorComponent.hpp"
#include "glvm/Components/ColliderComponent.hpp"
#include "glvm/Components/CrosshairComponent.hpp"
#include "glvm/Components/InventoryComponent.hpp"
#include "glvm/Components/ItemComponent.hpp"
#include "glvm/Components/RigidBodyComponent.hpp"
#include "glvm/Components/TransformComponent.hpp"
#include "glvm/EntityManager.hpp"
#include "glvm/EventsStack.hpp"
#include "glvm/ISystem.hpp"

#include <climits>

namespace GLVM::ecs {
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
        //
        //			arch::ProjectileBundle*             projectileBundles =
        // nullptr;
    } componentsView;

    arch::componentMask inventoryRequiredMask =
        (1ull << ecs::arch::ComponentsIndices::INVENTORY_COMPONENT);

    arch::componentMask itemRequiredMask =
        (1ul << arch::ComponentsIndices::TRANSFORM_COMPONENT)
        | (1ul << arch::ComponentsIndices::ITEM_COMPONENT)
        | (1ul << arch::ComponentsIndices::MESH_COMPONENT)
        | (1ul << arch::ComponentsIndices::MATERIAL_COMPONENT)
        | (1ul << arch::ComponentsIndices::COLLIDER_COMPONENT)
        | (1ul << arch::ComponentsIndices::COLLIDER_FLAGS_COMPONENT);

    arch::componentMask crosshairRequiredMask =
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
} // namespace GLVM::ecs

#endif
