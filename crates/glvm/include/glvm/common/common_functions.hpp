#pragma once

#include "glvm/archetype_ecs/arch_ecs_types.hpp"
#include "glvm/archetype_ecs/arch_ecs_world.hpp"
#include "glvm/archetype_ecs/archetype_entity_manager.hpp"
#include "glvm/archetypes/projectile_archetype.hpp"
#include "glvm/components/vertex_component.hpp"
#include "glvm/components_full_set.hpp"
#include "glvm/globals.hpp"
#include "glvm/vertex_math.hpp"
#include "glvm/typenames.hpp"

namespace glvm::core {
bool BoxCollider(
    const Vector<float, 3> backtrackingPosition,
    const Vector<float, 3> comparedPosition,
    const float backtrackingScale,
    const float comparedScale,
    const core::MeshAxisMaxAbsoluteValues& backtrackingMeshAxisMaxAbsoluteValues,
    const core::MeshAxisMaxAbsoluteValues& comparedMeshAxisMaxAbsoluteValues
);

std::vector<Vector<float, 3>> computeBoxCornerBoundPoints(
    const core::MeshAxisMaxAbsoluteValues entityChunkBounds,
    Vector<float, 3> entityPosition,
    const float scale
);

template<typename T>
bool isExist(const std::vector<T>& array, const T& element) {
    for (uint32_t i0 = 0; i0 < array.size(); ++i0) {
        if (element == array[i0]) {
            return true;
        }
    }

    return false;
}

void setMeshBounds(MeshAxisLimitingValues meshAxisLimitingValues);
void CreateProjectile(
    const Vector<float, 3>& projectilePosition,
    const Vector<float, 3>& projectileForward,
    const ecs::components::MeshHandle& meshHandle,
    const ecs::components::material& material,
    const ecs::components::damage& damage,
    const ecs::arch::EntityLocation& projectileLocation
);
}; // namespace glvm::core
