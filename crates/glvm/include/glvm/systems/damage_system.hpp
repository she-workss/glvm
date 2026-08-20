#pragma once

#include "glvm/archetype_ecs/archetype_interface.hpp"
#include "glvm/components/attack_component.hpp"
#include "glvm/components/damage_component.hpp"
#include "glvm/components/health_component.hpp"
#include "glvm/entity_manager.hpp"
#include "glvm/i_system.hpp"

namespace glvm::ecs {
class DamageSystem: public ISystem {
public:
    void Update() override;

    float deltaTime;

    uint32_t cachedAttackableArchetypesNumber = 0;
    uint32_t cachedFontArchetypesNumber = 0;

    struct ArchView {
        arch::Archetype* cachedAttackableArchetypes[32];
        arch::Archetype* cachedFontArchetypes[32];
    } archView;

    struct ComponentsView {
        ecs::components::attack* attackableAttacks = nullptr;
        ecs::components::health* attackableHealth = nullptr;
        ecs::components::font* attackableFonts = nullptr;

        ecs::components::font* fonts = nullptr;
    } componentsView;

    uint64_t attackableRequiredMask =
        (1ul << arch::ComponentsIndices::ATTACK_COMPONENT)
        | (1ul << arch::ComponentsIndices::HEALTH_COMPONENT)
        | (1ul << arch::ComponentsIndices::FONT_COMPONENT);

    uint64_t fontRequiredMask =
        (1ull << ecs::arch::ComponentsIndices::FONT_COMPONENT);
};
} // namespace glvm::ecs
