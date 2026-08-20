#include "glvm/systems/damage_system.hpp"

#include "glvm/archetype_ecs/arch_ecs_types.hpp"
#include "glvm/archetype_ecs/arch_ecs_world.hpp"
#include "glvm/archetype_ecs/archetype_entity_manager.hpp"
#include "glvm/archetypes/enemy_archetype.hpp"
#include "glvm/archetypes/player_archetype.hpp"
#include "glvm/archetypes/projectile_archetype.hpp"
#include "glvm/archetypes/static_mesh_archetype.hpp"
#include "glvm/components/attack_component.hpp"
#include "glvm/components/font_component.hpp"

namespace glvm::ecs {
void DamageSystem::Update() {
    namespace cm = glvm::ecs::components;

    cachedAttackableArchetypesNumber = 0;
    arch::world.searchCacheArchetypes(
        attackableRequiredMask,
        archView.cachedAttackableArchetypes,
        cachedAttackableArchetypesNumber
    );

    for (uint32_t x = 0; x < cachedAttackableArchetypesNumber; ++x) {
        arch::Archetype* arch = archView.cachedAttackableArchetypes[x];
        componentsView.attackableAttacks =
            (ecs::components::attack*)
                arch->components[arch::ComponentsIndices::ATTACK_COMPONENT];
        componentsView.attackableHealth =
            (ecs::components::health*)
                arch->components[arch::ComponentsIndices::HEALTH_COMPONENT];
        componentsView.attackableFonts =
            (ecs::components::font*)
                arch->components[arch::ComponentsIndices::FONT_COMPONENT];

        for (unsigned int i = 0; i < arch->entityCount; ++i) {
            uint64_t entity = arch->entities[i];
            if (&componentsView.attackableHealth[i] != nullptr
                && &componentsView.attackableAttacks[i] != nullptr) {
                cm::health& healthComponent =
                    componentsView.attackableHealth[i];
                cm::attack& attackComponent =
                    componentsView.attackableAttacks[i];

                healthComponent.currentHealth -= attackComponent.damage;
                attackComponent.damage = 0;
                if (healthComponent.currentHealth <= 0) {
                    ecs::arch::ArchetypeEntityManager* archEntityManager =
                        ecs::arch::ArchetypeEntityManager::getInstance();
                    archEntityManager->removeEntity(entity);
                    arch::world.removeEntity(entity);
                }

                cm::font& fontComponent = componentsView.attackableFonts[i];
                fontComponent.font_string.clear();
                fontComponent.font_string.push_back('4');
                fontComponent.font_string.push_back('0');
                fontComponent.lifeTime = 0;
                fontComponent.removeble = true;
            }
        }
    }

    cachedFontArchetypesNumber = 0;
    arch::world.searchCacheArchetypes(
        fontRequiredMask,
        archView.cachedFontArchetypes,
        cachedFontArchetypesNumber
    );

    for (uint32_t x = 0; x < cachedFontArchetypesNumber; ++x) {
        arch::Archetype* arch = archView.cachedFontArchetypes[x];
        componentsView.fonts =
            (ecs::components::font*)
                arch->components[arch::ComponentsIndices::FONT_COMPONENT];

        for (unsigned int i = 0; i < arch->entityCount; ++i) {
            if (componentsView.fonts) {
                cm::font& fontComponent = componentsView.fonts[i];
                if (fontComponent.removeble) {
                    fontComponent.lifeTime += deltaTime;
                }
                if (fontComponent.lifeTime >= 1.5) {}
            }
        }
    }
}
} // namespace glvm::ecs
