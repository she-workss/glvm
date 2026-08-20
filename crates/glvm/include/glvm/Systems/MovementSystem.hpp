#pragma once

#include "glvm/ArchetypeECS/ArchetypeEntityManager.hpp"
#include "glvm/ArchetypeECS/ArchetypeInterface.hpp"
#include "glvm/Archetypes/PlayerArchetype.hpp"
#include "glvm/ComponentManager.hpp"
#include "glvm/Components/ColliderFlagsComponent.hpp"
#include "glvm/Components/ItemComponent.hpp"
#include "glvm/Components/MoveComponent.hpp"
#include "glvm/Components/RigidBodyComponent.hpp"
#include "glvm/Components/SpotLightComponent.hpp"
#include "glvm/Components/TransformComponent.hpp"
#include "glvm/Components/ViewComponent.hpp"
#include "glvm/EntityManager.hpp"
#include "glvm/Event.hpp"
#include "glvm/EventsStack.hpp"
#include "glvm/Globals.hpp"
#include "glvm/ISoundEngine.hpp"
#include "glvm/ISystem.hpp"
#include "glvm/Vector.hpp"
#include "glvm/VertexMath.hpp"

namespace glvm::ecs {
class CMovementSystem: public ISystem {
public:
    float deltaFrameTime;
    float gravity;
    core::CStack& inputStack;
    float prev_delta_x = 0.0f;
    float prev_X = 0.0f;
    float current_X = 0.0f;
    vec3 prev_forward;

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

    arch::componentMask playerRequiredMask =
        (1ull << ecs::arch::ComponentsIndices::PLAYER_TAG_COMPONENT);
    arch::componentMask rigidBodyRequiredMask =
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
