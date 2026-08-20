#pragma once

#include "glvm/archetype_ecs/archetype_interface.hpp"
#include "glvm/common/common_functions.hpp"
#include "glvm/components/collider_component.hpp"
#include "glvm/components/collider_flags_component.hpp"
#include "glvm/components/move_component.hpp"
#include "glvm/components/transform_component.hpp"
#include "glvm/components/vertex_component.hpp"
#include "glvm/event.hpp"
#include "glvm/globals.hpp"
#include "glvm/i_system.hpp"
#include "glvm/vertex_math.hpp"

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

    uint64_t requiredMask =
        (1ul << arch::ComponentsIndices::COLLIDER_COMPONENT)
        | (1ul << arch::ComponentsIndices::COLLIDER_FLAGS_COMPONENT)
        | (1ul << arch::ComponentsIndices::TRANSFORM_COMPONENT)
        | (1ul << arch::ComponentsIndices::MESH_COMPONENT);

    CCollisionSystem(core::CStack& _input_Stack) : Input_Stack_(_input_Stack) {}

    void Update() override;
    bool UpperActorCheck(
        Vector<float, 3> backtrackingPosition,
        Vector<float, 3> comparedPosition,
        float backtrackingScale,
        float comparedScale,
        components::MeshHandle backtrackingMeshHandle,
        components::MeshHandle comparedMeshHandle
    );
};
} // namespace glvm::ecs
