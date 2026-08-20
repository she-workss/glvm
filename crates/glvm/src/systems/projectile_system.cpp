#include "glvm/systems/projectile_system.hpp"

#include "glvm/archetype_ecs/arch_ecs_types.hpp"
#include "glvm/archetype_ecs/arch_ecs_utils.hpp"
#include "glvm/archetype_ecs/arch_ecs_world.hpp"
#include "glvm/archetype_ecs/archetype_entity_manager.hpp"
#include "glvm/archetype_ecs/archetype_interface.hpp"
#include "glvm/archetypes/enemy_archetype.hpp"
#include "glvm/archetypes/player_archetype.hpp"
#include "glvm/archetypes/projectile_archetype.hpp"
#include "glvm/components/actor_component.hpp"
#include "glvm/components/attack_component.hpp"
#include "glvm/components/collider_component.hpp"
#include "glvm/components/collider_flags_component.hpp"
#include "glvm/components/controller_component.hpp"
#include "glvm/components/damage_component.hpp"
#include "glvm/components/health_component.hpp"
#include "glvm/components/material_component.hpp"
#include "glvm/components/point_light_component.hpp"
#include "glvm/components/projectile_bundle.hpp"
#include "glvm/components/projectile_component.hpp"
#include "glvm/components/transform_component.hpp"
#include "glvm/components/vertex_component.hpp"
#include "glvm/texture.hpp"
#include "glvm/vertex_math.hpp"

#include <cstdint>

namespace glvm::ecs {
CProjectileSystem::CProjectileSystem(core::CStack& inputStack) :
    inputStack(inputStack) {}

void CProjectileSystem::Update() {
    namespace cm = glvm::ecs::components;
    namespace arch = glvm::ecs::arch;

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

    // Iterate on every player and create projectile if "LMB pressed" event
    // found.
    for (unsigned int i = 0; i < archView.playerCachedArchetype->entityCount;
         ++i) {
        cm::beholder* playerView = &componentsView.playerViews[i];
        cm::transform* playerTransform = &componentsView.playerTransforms[i];
        const uint32_t maxEventNumber = 6;
        for (uint32_t n = 0; n < maxEventNumber; ++n) {
            if (!isInventoryOpened
                && inputStack.SearchElement(core::EEvents::eMOUSE_LEFT_BUTTON)
                    == core::EEvents::eMOUSE_LEFT_BUTTON) {
                if (projectileCooldown <= 0) {
                    ecs::components::MeshHandle meshHandle {};
                    const uint32_t sphereMeshHandleIndex = 2;
                    if (meshHandlers.size() > 2) {
                        meshHandle = meshHandlers[sphereMeshHandleIndex];
                    }

                    ecs::TextureHandle textureHandle {};
                    const uint32_t grayTextureHandle = 2;
                    if (textureHandlers.size() > 2) {
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
                    uint64_t projectileEntity =
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
                        "../../../examples/assets/sounds/pistol.wav",
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

    // Update position of every projectile.
    for (unsigned int x = 0; x < archView.projectileArchetype->entityCount;
         ++x) {
        cm::transform* projectileTransform =
            &componentsView.projectileTransforms[x];
        projectileTransform->position +=
            Normalize(projectileTransform->forward) * cameraSpeed * 2.5;
    }
    // Iterate every projectile, check for collisions with another entities and
    // update damage info if collided entity has attack component.
    for (unsigned int i = 0; i < archView.projectileArchetype->entityCount;
         ++i) {
        cm::colliderFlags* projectileColliderFlags =
            &componentsView.projectileColliderFlags[i];
        cm::health* projectileHealth = &componentsView.projectileHealth[i];
        [[maybe_unused]] cm::attack* projectileAttack =
            &componentsView.projectileAttacks[i];
        const uint8_t wallCollisionBit = 1;
        const uint8_t groundCollistionBit = (1 << 1);
        if ((projectileColliderFlags->flags & wallCollisionBit)
            || (projectileColliderFlags->flags & groundCollistionBit)) {
            cm::damage* projectileDamage =
                &componentsView.projectileBundles[i].damage;
            cm::collider* projectileCollider =
                &componentsView.projectileColliders[i];
            for (unsigned int j = 0; j < projectileCollider->colliders.size();
                 ++j) {
                unsigned int collidedEntity = projectileCollider->colliders[j];

                arch::EntityLocation collidedEntityLocation =
                    arch::world.entityLocations[arch::getId(collidedEntity)];
                uint64_t requiredMask =
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
                }
            }
        }
    }
}
} // namespace glvm::ecs
