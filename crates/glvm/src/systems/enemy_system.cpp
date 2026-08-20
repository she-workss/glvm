#include "glvm/systems/enemy_system.hpp"

#include "glvm/archetype_ecs/arch_ecs_world.hpp"
#include "glvm/archetype_ecs/archetype_entity_manager.hpp"
#include "glvm/archetypes/enemy_archetype.hpp"
#include "glvm/archetypes/player_archetype.hpp"
#include "glvm/archetypes/projectile_archetype.hpp"
#include "glvm/components/actor_component.hpp"
#include "glvm/components/damage_component.hpp"
#include "glvm/components/material_component.hpp"
#include "glvm/components/projectile_bundle.hpp"
#include "glvm/texture.hpp"

#include <cstdint>

namespace glvm::ecs {
void EnemySystem::Update() {
    namespace arch = glvm::ecs::arch;

    playerArchetypesNumber = 0;
    arch::world.searchCacheArchetypes(
        playerRequiredMask,
        &archView.playerCachedArchetype,
        playerArchetypesNumber
    );
    componentsView.playerTransforms =
        (ecs::components::transform*)archView.playerCachedArchetype
            ->components[arch::ComponentsIndices::TRANSFORM_COMPONENT];

    enemyArchetypesNumber = 0;
    arch::world.searchCacheArchetypes(
        enemyRequiredMask,
        &archView.enemyCachedArchetype,
        enemyArchetypesNumber
    );
    componentsView.enemyTransforms =
        (ecs::components::transform*)archView.enemyCachedArchetype
            ->components[arch::ComponentsIndices::TRANSFORM_COMPONENT];
    componentsView.enemyStates =
        (ecs::components::state*)archView.enemyCachedArchetype
            ->components[arch::ComponentsIndices::STATE_COMPONENT];
    componentsView.enemies =
        (ecs::components::enemy*)archView.enemyCachedArchetype
            ->components[arch::ComponentsIndices::ENEMY_COMPONENT];

    projectileArchetypesNumber = 0;
    ecs::arch::world.searchCacheArchetypes(
        projectileRequiredMask,
        &archView.projectileArchetype,
        projectileArchetypesNumber
    );

    for (uint32_t j = 0; j < archView.playerCachedArchetype->entityCount; ++j) {
        components::transform* playerTransformComponent =
            &componentsView.playerTransforms[j];
        for (unsigned int i = 0; i < archView.enemyCachedArchetype->entityCount;
             ++i) {
            components::transform* enemyTransformComponent =
                &componentsView.enemyTransforms[i];
            components::state* stateEnemyComponent =
                &componentsView.enemyStates[i];
            components::enemy* enemyComponent = &componentsView.enemies[i];

            Vector<float, 3> distance = playerTransformComponent->position
                - enemyTransformComponent->position;
            float cameraSpeed = 5.5f * deltaFrameTime;

            if (projectileCooldown > 0) {
                projectileCooldown -= cameraSpeed;
            }
            if (distance.Length() > enemyComponent->detectRadius
                && stateEnemyComponent->state == core::States::ATTACK) {
                float deltaLength =
                    distance.Length() - enemyComponent->detectRadius;
                Vector<float, 3> enemyMove =
                    distance * (deltaLength / distance.Length());

                enemyTransformComponent->position += enemyMove;
            }

            if (distance.Length() <= enemyComponent->detectRadius) {
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
                        enemyTransformComponent->position,
                        playerTransformComponent->position
                            - enemyTransformComponent->position,
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
                    projectileCooldown = 5.0;
                }

                stateEnemyComponent->state = core::States::ATTACK;
            }
        }
    }
}
} // namespace glvm::ecs
