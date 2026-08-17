// This file is part of Game Loop Versatile Modules (GLVM)
// Copyright © 2024 Maksim Manokhin a.k.a. Yuriorkis_Scream. Contacts:
// <fellfrostqtw@gmail.com> Author: Maksim Manokhin a.k.a. Yuriorkis_Scream
// License: http://opensource.org/licenses/MIT

#include "glvm/Systems/DamageSystem.hpp"

#include "glvm/ArchetypeECS/ArchECS_Types.hpp"
#include "glvm/ArchetypeECS/ArchECS_World.hpp"
#include "glvm/ArchetypeECS/ArchetypeEntityManager.hpp"
#include "glvm/Archetypes/EnemyArchetype.hpp"
#include "glvm/Archetypes/PlayerArchetype.hpp"
#include "glvm/Archetypes/ProjectileArchetype.hpp"
#include "glvm/Archetypes/StaticMeshArchetype.hpp"
#include "glvm/Components/AttackComponent.hpp"
#include "glvm/Components/FontComponent.hpp"

namespace GLVM::ecs {
void DamageSystem::Update() {
    namespace cm = GLVM::ecs::components;

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
            //				unsigned int entity = linkedEntities[i];
            arch::entity entity = arch->entities[i];
            if (&componentsView.attackableHealth[i] != nullptr
                && &componentsView.attackableAttacks[i] != nullptr) {
                cm::health& healthComponent =
                    componentsView.attackableHealth[i];
                cm::attack& attackComponent =
                    componentsView.attackableAttacks[i];

                //					std::cout << "damage: " <<
                // attackComponent.damage << std::endl;
                healthComponent.currentHealth -= attackComponent.damage;
                attackComponent.damage = 0;
                //					std::cout << "current health: " <<
                // healthComponent.currentHealth << std::endl;
                //				componentManager->RemoveComponent<cm::attack>(entity);
                if (healthComponent.currentHealth <= 0) {
                    //					std::cout << "remove entity: " << entity
                    //<< std::endl;
                    // entityManager->RemoveEntity(entity, componentManager);
                    ecs::arch::ArchetypeEntityManager* archEntityManager =
                        ecs::arch::ArchetypeEntityManager::getInstance();
                    archEntityManager->removeEntity(entity);
                    arch::world.removeEntity(entity);
                }

                cm::font& fontComponent = componentsView.attackableFonts[i];
                fontComponent.font_string.clear();
                fontComponent.font_string.Push('4');
                fontComponent.font_string.Push('0');
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
                //			std::cout << "lifeTime" << fontComponent->lifeTime
                //<< std::endl;
                if (fontComponent.lifeTime >= 1.5) {
                    //				std::cout << "lifetime: " <<
                    // fontComponent->lifeTime << std::endl;
                    //					componentManager->RemoveComponent<cm::font>(entity);
                }
            }
        }
    }
}
} // namespace GLVM::ecs
