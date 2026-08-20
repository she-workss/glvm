#pragma once

#include "glvm/ArchetypeECS/ArchetypeInterface.hpp"
#include "glvm/Common/CommonFunctions.hpp"
#include "glvm/Components/ColliderComponent.hpp"
#include "glvm/Components/ColliderFlagsComponent.hpp"
#include "glvm/Components/MoveComponent.hpp"
#include "glvm/Components/TransformComponent.hpp"
#include "glvm/Components/VertexComponent.hpp"
#include "glvm/Event.hpp"
#include "glvm/Globals.hpp"
#include "glvm/ISystem.hpp"
#include "glvm/VertexMath.hpp"

#include <cstdint>

namespace glvm::ecs {
class CCollisionSystem: public ISystem {
public:
    float fDelta_Time_;
    float gravity;
    bool isInventoryOpened;
    bool* isItemDraged;
    bool isLeftMouseButtonPressed;
    bool* isLeftMouseButtonReleased;
    core::CStack& Input_Stack_;
    arch::Archetype* cachedArchetypes[32];
    uint32_t cachedArchetypesNumber = 0;

    struct CollisionComponentsView {
        components::transform* backtrackingTransforms = nullptr;
        components::collider* backtrackingColliders = nullptr;
        components::colliderFlags* backtrackingColliderFlags = nullptr;
        components::mesh* backtrackingMeshes = nullptr;
        components::move* backtrackingMove = nullptr;
        components::transform* comparedTransforms = nullptr;
        components::mesh* comparedMeshes = nullptr;
        components::move* comparedMove = nullptr;
    } view;

    arch::componentMask requiredMask =
        (1ul << arch::ComponentsIndices::COLLIDER_COMPONENT)
        | (1ul << arch::ComponentsIndices::COLLIDER_FLAGS_COMPONENT)
        | (1ul << arch::ComponentsIndices::TRANSFORM_COMPONENT)
        | (1ul << arch::ComponentsIndices::MESH_COMPONENT);

    CCollisionSystem(core::CStack& _input_Stack) : Input_Stack_(_input_Stack) {}

    void Update() override;
    bool UpperActorCheck(
        vec3 backtrackingPosition,
        vec3 comparedPosition,
        float backtrackingScale,
        float comparedScale,
        components::MeshHandle backtrackingMeshHandle,
        components::MeshHandle comparedMeshHandle
    );
};
} // namespace glvm::ecs
