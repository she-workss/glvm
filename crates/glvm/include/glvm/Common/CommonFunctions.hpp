// This file is part of Game Loop Versatile Modules (GLVM)
// Copyright © 2024 Maksim Manokhin a.k.a. Yuriorkis_Scream. Contacts:
// <fellfrostqtw@gmail.com> Author: Maksim Manokhin a.k.a. Yuriorkis_Scream
// License: http://opensource.org/licenses/MIT

#ifndef COMMON_FUNCTIONS_HPP
#define COMMON_FUNCTIONS_HPP

#include "glvm/ArchetypeECS/ArchECS_Types.hpp"
#include "glvm/ArchetypeECS/ArchECS_World.hpp"
#include "glvm/ArchetypeECS/ArchetypeEntityManager.hpp"
#include "glvm/Archetypes/ProjectileArchetype.hpp"
#include "glvm/Components/VertexComponent.hpp"
#include "glvm/ComponentsFullSet.hpp"
#include "glvm/Globals.hpp"
#include "glvm/VertexMath.hpp"
#include "glvm/typenames.hpp"

namespace GLVM::core {
bool BoxCollider(
    const vec3 backtrackingPosition,
    const vec3 comparedPosition,
    const float backtrackingScale,
    const float comparedScale,
    const core::MeshAxisMaxAbsoluteValues& backtrackingMeshAxisMaxAbsoluteValues,
    const core::MeshAxisMaxAbsoluteValues& comparedMeshAxisMaxAbsoluteValues
);

core::vector<vec3> computeBoxCornerBoundPoints(
    const core::MeshAxisMaxAbsoluteValues entityChunkBounds,
    vec3 entityPosition,
    const float scale
);

template<typename T>
bool isExist(const core::vector<T>& array, const T& element) {
    for (u32 i0 = 0; i0 < array.GetSize(); ++i0) {
        if (element == array[i0]) {
            return true;
        }
    }

    return false;
}

void setMeshBounds(MeshAxisLimitingValues meshAxisLimitingValues);
void CreateProjectile(
    const vec3& projectilePosition,
    const vec3& projectileForward,
    const ecs::components::MeshHandle& meshHandle,
    const ecs::components::material& material,
    const ecs::components::damage& damage,
    const ecs::arch::EntityLocation& projectileLocation
);
}; // namespace GLVM::core

#endif
