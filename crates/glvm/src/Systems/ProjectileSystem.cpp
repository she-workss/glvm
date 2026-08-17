// This file is part of Game Loop Versatile Modules (GLVM)
// Copyright © 2024 Maksim Manokhin a.k.a. Yuriorkis_Scream. Contacts:
// <fellfrostqtw@gmail.com> Author: Maksim Manokhin a.k.a. Yuriorkis_Scream
// License: http://opensource.org/licenses/MIT

#include "glvm/Systems/ProjectileSystem.hpp"

#include "glvm/ArchetypeECS/ArchECS_Types.hpp"
#include "glvm/ArchetypeECS/ArchECS_Utils.hpp"
#include "glvm/ArchetypeECS/ArchECS_World.hpp"
#include "glvm/ArchetypeECS/ArchetypeEntityManager.hpp"
#include "glvm/ArchetypeECS/ArchetypeInterface.hpp"
#include "glvm/Archetypes/EnemyArchetype.hpp"
#include "glvm/Archetypes/PlayerArchetype.hpp"
#include "glvm/Archetypes/ProjectileArchetype.hpp"
#include "glvm/Components/ActorComponent.hpp"
#include "glvm/Components/AttackComponent.hpp"
#include "glvm/Components/ColliderComponent.hpp"
#include "glvm/Components/ColliderFlagsComponent.hpp"
#include "glvm/Components/ControllerComponent.hpp"
#include "glvm/Components/DamageComponent.hpp"
#include "glvm/Components/HealthComponent.hpp"
#include "glvm/Components/MaterialComponent.hpp"
#include "glvm/Components/PointLightComponent.hpp"
#include "glvm/Components/ProjectileBundle.hpp"
#include "glvm/Components/ProjectileComponent.hpp"
#include "glvm/Components/TransformComponent.hpp"
#include "glvm/Components/VertexComponent.hpp"
#include "glvm/Texture.hpp"
#include "glvm/VertexMath.hpp"

#include <cstdint>

namespace GLVM::ecs {
CProjectileSystem::CProjectileSystem(core::CStack& inputStack) :
    inputStack(inputStack) {}

void CProjectileSystem::Update() {
    namespace cm = GLVM::ecs::components;
    namespace arch = GLVM::ecs::arch;

    float cameraSpeed = 5.5f * deltaFrameTime;

    playerArchetypesNumber = 0;
    arch::world.searchCacheArchetypes(
        playerRequiredMask,
        &archView.playerCachedArchetype,
        playerArchetypesNumber
    );
    componentsView.playerTransforms =
        (ecs::components::transform*)archView.playerCachedArchetype
            ->components[arch::ComponentsIndices::TRANSFORM_COMPONENT];
    componentsView.playerViews =
        (ecs::components::beholder*)archView.playerCachedArchetype
            ->components[arch::ComponentsIndices::VIEW_COMPONENT];

    projectileArchetypesNumber = 0;
    ecs::arch::world.searchCacheArchetypes(
        projectileRequiredMask,
        &archView.projectileArchetype,
        projectileArchetypesNumber
    );

    if (projectileCooldown > 0) {
        projectileCooldown -= cameraSpeed;
    }

    /// Iterate on every player and create projectile if "LMB pressed" event found
    for (unsigned int i = 0; i < archView.playerCachedArchetype->entityCount;
         ++i) {
        cm::beholder* playerView = &componentsView.playerViews[i];
        cm::transform* playerTransform = &componentsView.playerTransforms[i];
        const u32 maxEventNumber = 6;
        for (u32 n = 0; n < maxEventNumber; ++n) {
            if (!isInventoryOpened
                && inputStack.SearchElement(core::EEvents::eMOUSE_LEFT_BUTTON)
                    == core::EEvents::eMOUSE_LEFT_BUTTON) {
                if (projectileCooldown <= 0) {
                    ecs::components::MeshHandle meshHandle {};
                    const u32 sphereMeshHandleIndex = 2;
                    if (meshHandlers.GetSize() > 2) {
                        meshHandle = meshHandlers[sphereMeshHandleIndex];
                    }

                    ecs::TextureHandle textureHandle {};
                    const u32 grayTextureHandle = 2;
                    if (textureHandlers.GetSize() > 2) {
                        textureHandle = textureHandlers[grayTextureHandle];
                    }

                    const components::material material = {
                        .diffuseTextureID_ = textureHandle,
                        .specularTextureID_ = textureHandle,
                        .ambient = {0.05f, 0.05f, 0.05f},
                        .shininess = 128.0f * 0.078125f
                    };

                    const components::damage damage = {
                        .maximumDamage = 40,
                        .minimumDamage = 20,
                        .criticalHitRate = 0,
                        .criticalModifier = 0
                    };

                    ecs::arch::ArchetypeEntityManager* archEntityManager =
                        ecs::arch::ArchetypeEntityManager::getInstance();
                    ecs::arch::entity projectileEntity =
                        archEntityManager->createEntity();
                    ecs::arch::world.addEntityToArchetype(
                        projectileEntity,
                        archView.projectileArchetype
                    );
                    ecs::arch::EntityLocation projectileLocation =
                        ecs::arch::world
                            .entityLocations[ecs::arch::getId(projectileEntity)];

                    core::CreateProjectile(
                        playerTransform->position,
                        playerView->forward,
                        meshHandle,
                        material,
                        damage,
                        projectileLocation
                    );

                    soundEngine->CreateSoundSample(
                        "../../../assets/sounds/laser2.wav",
                        5,
                        22050,
                        0.05
                    );
                    projectileCooldown = 2.0;
                }
            }
        }
    }

    projectileArchetypesNumber = 0;
    ecs::arch::world.searchCacheArchetypes(
        projectileRequiredMask,
        &archView.projectileArchetype,
        projectileArchetypesNumber
    );

    componentsView.projectileTransforms =
        (ecs::components::transform*)archView.projectileArchetype
            ->components[arch::ComponentsIndices::TRANSFORM_COMPONENT];
    componentsView.projectileColliderFlags =
        (ecs::components::colliderFlags*)archView.projectileArchetype
            ->components[arch::ComponentsIndices::COLLIDER_FLAGS_COMPONENT];
    componentsView.projectileColliders =
        (ecs::components::collider*)archView.projectileArchetype
            ->components[arch::ComponentsIndices::COLLIDER_COMPONENT];
    componentsView.projectileBundles =
        (arch::ProjectileBundle*)archView.projectileArchetype
            ->components[arch::ComponentsIndices::PROJECTILE_BUNDLE_COMPONENT];
    componentsView.projectileHealth =
        (ecs::components::health*)archView.projectileArchetype
            ->components[arch::ComponentsIndices::HEALTH_COMPONENT];
    componentsView.projectileAttacks =
        (ecs::components::attack*)archView.projectileArchetype
            ->components[arch::ComponentsIndices::ATTACK_COMPONENT];

    /// Update position of every projectile
    for (unsigned int x = 0; x < archView.projectileArchetype->entityCount;
         ++x) {
        cm::transform* projectileTransform =
            &componentsView.projectileTransforms[x];
        projectileTransform->position +=
            Normalize(projectileTransform->forward) * cameraSpeed * 2.5;
    }

    /// Iterate every projectile, check for collistions with another entities
    /// and update damage info if collided entity has attack component
    for (unsigned int i = 0; i < archView.projectileArchetype->entityCount;
         ++i) {
        //			const u32 entity = archView.projectileArchetype->entities[i];
        cm::colliderFlags* projectileColliderFlags =
            &componentsView.projectileColliderFlags[i];
        cm::health* projectileHealth = &componentsView.projectileHealth[i];
        [[maybe_unused]] cm::attack* projectileAttack =
            &componentsView.projectileAttacks[i];
        const u8 wallCollisionBit = 1;
        const u8 groundCollistionBit = (1 << 1);
        if ((projectileColliderFlags->flags & wallCollisionBit)
            || (projectileColliderFlags->flags & groundCollistionBit)) {
            cm::damage* projectileDamage =
                &componentsView.projectileBundles[i].damage;
            cm::collider* projectileCollider =
                &componentsView.projectileColliders[i];
            for (unsigned int j = 0;
                 j < projectileCollider->colliders.GetSize();
                 ++j) {
                unsigned int collidedEntity = projectileCollider->colliders[j];

                arch::EntityLocation collidedEntityLocation =
                    arch::world.entityLocations[arch::getId(collidedEntity)];
                arch::componentMask requiredMask =
                    (1ul << arch::ComponentsIndices::HEALTH_COMPONENT)
                    | (1ul << arch::ComponentsIndices::ATTACK_COMPONENT);

                if ((collidedEntityLocation.arch != nullptr)
                    && (collidedEntityLocation.arch->mask & requiredMask)
                        == requiredMask) {
                    ecs::components::attack* attacks =
                        (ecs::components::attack*)
                            collidedEntityLocation.arch->components
                                [arch::ComponentsIndices::ATTACK_COMPONENT];
                    attacks[collidedEntityLocation.index].damage =
                        projectileDamage->maximumDamage;

                    projectileHealth->currentHealth = 0;
                    //						projectileAttack->damage = 100;
                    // ecs::arch::ArchetypeEntityManager* archEntityManager =
                    // ecs::arch::ArchetypeEntityManager::getInstance();
                    // archEntityManager->removeEntity( entity );
                    // arch::world.removeEntity( entity );
                }
            }
            //                pEntity_Manager->RemoveEntity(uiEntity_refProjectile,
            //                pComponent_Manager);
        }
    }
}
} // namespace GLVM::ecs
