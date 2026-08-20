#pragma once

#include "glvm/archetype_ecs/arch_ecs_world.hpp"
#include "glvm/archetype_ecs/archetype_interface.hpp"
#include "glvm/common/common_functions.hpp"
#include "glvm/globals.hpp"
#include "glvm/i_system.hpp"
#include "glvm/vertex_math.hpp"

#include <vector>

namespace glvm::ecs {

class SpatialGridSystem: public ISystem {
    arch::Archetype* cachedArchetypes[32];
    uint32_t cachedArchetypesNumber = 0;
    bool isInitialized = false;

    struct SpatialGridComponentsView {
        components::transform* transforms = nullptr;
        components::mesh* meshes = nullptr;
    } view;

    uint64_t requiredMask =
        (1ul << arch::ComponentsIndices::COLLIDER_COMPONENT)
        | (1ul << arch::ComponentsIndices::COLLIDER_FLAGS_COMPONENT)
        | (1ul << arch::ComponentsIndices::TRANSFORM_COMPONENT)
        | (1ul << arch::ComponentsIndices::MESH_COMPONENT);

    void Update() override;
};

}; // namespace glvm::ecs
