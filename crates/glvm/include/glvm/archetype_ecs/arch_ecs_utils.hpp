#pragma once

#include "glvm/archetype_ecs/arch_ecs_types.hpp"
#include "glvm/archetype_ecs/archetype_interface.hpp"
#include "glvm/archetypes/enemy_archetype.hpp"
#include "glvm/archetypes/player_archetype.hpp"
#include "glvm/components_full_set.hpp"

namespace glvm::ecs::arch {
uint64_t makeEntity(uint32_t id_, uint32_t generation_);
uint32_t getId(uint64_t entity_);
uint32_t getGen(uint64_t entity_);
bool matchesRequiredMask(
    const uint64_t archetypeMask,
    const uint64_t& systemMask
);

template<typename T>
void unwrapArchetype(
    arch::Archetype* arch,
    uint64_t mask,
    void (*func)(T*)
) {
    switch (mask) {
        case arch::playerComponentMask:
            func(static_cast<arch::PlayerArchetype*>(arch));
            break;
        case arch::enemyComponentMask:
            func(static_cast<arch::EnemyArchetype*>(arch));
            break;
    }
}
}; // namespace glvm::ecs::arch
