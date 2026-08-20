#pragma once

#include "glvm/archetype_ecs/archetype_interface.hpp"
#include "glvm/component_manager.hpp"
#include "glvm/components/collider_component.hpp"
#include "glvm/components/collider_flags_component.hpp"
#include "glvm/components/transform_component.hpp"
#include "glvm/components/view_component.hpp"
#include "glvm/event.hpp"
#include "glvm/events_stack.hpp"
#include "glvm/i_system.hpp"

#include <vector>

namespace glvm::ecs {
class CPhysicsSystem: public ISystem {
public:
    float fAcceleration_of_Gravity_;
    float fDelta_Time_;
    float& gravity;
    core::CStack& Input_Stack_;

    uint32_t cachedArchetypesNumber = 0;

    struct ArchView {
        arch::Archetype* cachedArchetypes[32];
    } archView;

    struct ComponentsView {
        components::transform* transformsView = nullptr;
        components::move* movesView = nullptr;
        components::rigidBody* rigidBodiesView = nullptr;
        components::colliderFlags* colliderFlagsView = nullptr;
        components::collider* collidersView = nullptr;
        components::mesh* meshesView = nullptr;
    } componentsView;

    uint64_t requiredMask =
        (1ul << arch::ComponentsIndices::TRANSFORM_COMPONENT)
        | (1ul << arch::ComponentsIndices::MOVE_COMPONENT)
        | (1ul << arch::ComponentsIndices::RIGID_BODY_COMPONENT)
        | (1ul << arch::ComponentsIndices::COLLIDER_COMPONENT)
        | (1ul << arch::ComponentsIndices::MESH_COMPONENT);

    CPhysicsSystem(float& gravity_, core::CStack& _input_Stack) :
        gravity(gravity_),
        Input_Stack_(_input_Stack) {}

    // Set Y-axis of transform component of backtracking entity to upper Y-axis
    // of ground entity.
    void Gravity();

    // This update searching for referring to colliders entities and check their
    // transform components for collision, and if collision detected check if
    // backtracking entity had gravity component for call Gravity function.
    void Update() override;
    void Repel(
        components::transform& _transform_Component,
        float& _fDelta_Time,
        components::beholder& _view_Component,
        core::CEvent& _event
    );
};
} // namespace glvm::ecs
