#pragma once

#include "glvm/ArchetypeECS/ArchetypeInterface.hpp"
#include "glvm/Components/AttackComponent.hpp"
#include "glvm/Components/DamageComponent.hpp"
#include "glvm/Components/HealthComponent.hpp"
#include "glvm/EntityManager.hpp"
#include "glvm/ISystem.hpp"

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

    arch::componentMask attackableRequiredMask =
        (1ul << arch::ComponentsIndices::ATTACK_COMPONENT)
        | (1ul << arch::ComponentsIndices::HEALTH_COMPONENT)
        | (1ul << arch::ComponentsIndices::FONT_COMPONENT);

    arch::componentMask fontRequiredMask =
        (1ull << ecs::arch::ComponentsIndices::FONT_COMPONENT);
};
} // namespace glvm::ecs
