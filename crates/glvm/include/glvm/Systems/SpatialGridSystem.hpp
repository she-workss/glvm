#pragma once

#include "glvm/ArchetypeECS/ArchECS_World.hpp"
#include "glvm/ArchetypeECS/ArchetypeInterface.hpp"
#include "glvm/Common/CommonFunctions.hpp"
#include "glvm/Globals.hpp"
#include "glvm/ISystem.hpp"
#include "glvm/Vector.hpp"
#include "glvm/VertexMath.hpp"

namespace glvm::ecs {

class SpatialGridSystem: public ISystem {
    arch::Archetype* cachedArchetypes[32];
    uint32_t cachedArchetypesNumber = 0;
    bool isInitialized = false;

    struct SpatialGridComponentsView {
        components::transform* transforms = nullptr;
        components::mesh* meshes = nullptr;
    } view;

    arch::componentMask requiredMask =
        (1ul << arch::ComponentsIndices::COLLIDER_COMPONENT)
        | (1ul << arch::ComponentsIndices::COLLIDER_FLAGS_COMPONENT)
        | (1ul << arch::ComponentsIndices::TRANSFORM_COMPONENT)
        | (1ul << arch::ComponentsIndices::MESH_COMPONENT);

    void Update() override;
};

}; // namespace glvm::ecs
