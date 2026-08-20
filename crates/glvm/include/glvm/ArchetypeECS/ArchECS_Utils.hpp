#pragma once

#include "glvm/ArchetypeECS/ArchECS_Types.hpp"
#include "glvm/ArchetypeECS/ArchetypeInterface.hpp"
#include "glvm/Archetypes/EnemyArchetype.hpp"
#include "glvm/Archetypes/PlayerArchetype.hpp"
#include "glvm/ComponentsFullSet.hpp"

namespace glvm::ecs::arch {
entity makeEntity(id id_, generation generation_);
id getId(entity entity_);
generation getGen(entity entity_);
bool matchesRequiredMask(
    const componentMask archetypeMask,
    const componentMask& systemMask
);

template<typename T>
void unwrapArchetype(
    arch::Archetype* arch,
    arch::componentMask mask,
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
