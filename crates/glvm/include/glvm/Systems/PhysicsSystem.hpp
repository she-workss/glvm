#pragma once

#include "glvm/ArchetypeECS/ArchetypeInterface.hpp"
#include "glvm/ComponentManager.hpp"
#include "glvm/Components/ColliderComponent.hpp"
#include "glvm/Components/ColliderFlagsComponent.hpp"
#include "glvm/Components/TransformComponent.hpp"
#include "glvm/Components/ViewComponent.hpp"
#include "glvm/Event.hpp"
#include "glvm/EventsStack.hpp"
#include "glvm/ISystem.hpp"
#include "glvm/Vector.hpp"

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

    arch::componentMask requiredMask =
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
