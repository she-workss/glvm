#pragma once

#include "glvm/archetype_ecs/archetype_entity_manager.hpp"
#include "glvm/archetype_ecs/archetype_interface.hpp"
#include "glvm/archetypes/player_archetype.hpp"
#include "glvm/component_manager.hpp"
#include "glvm/components/collider_flags_component.hpp"
#include "glvm/components/item_component.hpp"
#include "glvm/components/move_component.hpp"
#include "glvm/components/rigid_body_component.hpp"
#include "glvm/components/spot_light_component.hpp"
#include "glvm/components/transform_component.hpp"
#include "glvm/components/view_component.hpp"
#include "glvm/entity_manager.hpp"
#include "glvm/event.hpp"
#include "glvm/events_stack.hpp"
#include "glvm/globals.hpp"
#include "glvm/i_sound_engine.hpp"
#include "glvm/i_system.hpp"
#include "glvm/vertex_math.hpp"

#include <vector>

namespace glvm::ecs {
class CMovementSystem: public ISystem {
public:
    float deltaFrameTime;
    float gravity;
    core::CStack& inputStack;
    float prev_delta_x = 0.0f;
    float prev_X = 0.0f;
    float current_X = 0.0f;
    Vector<float, 3> prev_forward;

    uint32_t playerArchetypesNumber = 0;
    uint32_t rigidBodyContainedArchetypesNumber = 0;

    struct MovementArchView {
        arch::Archetype* playerCachedArchetype = nullptr;
        arch::Archetype* rigidBodyContainedArchetypesCache[32];
    } archView;

    struct MovementComponentsView {
        ecs::components::move* playerMoves = nullptr;
        ecs::components::beholder* playerViews = nullptr;
        ecs::components::colliderFlags* playerColliderFlags = nullptr;
        ecs::components::rigidBody* playerRigidBody = nullptr;

        // Components related to archetypes contains rigid.
        ecs::components::transform* transforms = nullptr;
        ecs::components::rigidBody* rigidBodies = nullptr;
        ecs::components::move* moves = nullptr;
        ecs::components::item* items = nullptr;
    } componentsView;

    uint64_t playerRequiredMask =
        (1ull << ecs::arch::ComponentsIndices::PLAYER_TAG_COMPONENT);
    uint64_t rigidBodyRequiredMask =
        (1ul << arch::ComponentsIndices::TRANSFORM_COMPONENT)
        | (1ul << arch::ComponentsIndices::RIGID_BODY_COMPONENT)
        | (1ul << arch::ComponentsIndices::MOVE_COMPONENT);

    CMovementSystem(core::CStack& inputStack);

    void Update();
    Vector<float, 3> CalculateVectorRL(components::beholder& beholder);
    Vector<float, 3> CalculateVectorFB(
        components::beholder& beholder,
        core::CEvent& event
    );
};
} // namespace glvm::ecs
