#include "glvm/Systems/EnemySystem.hpp"

#include "glvm/ArchetypeECS/ArchECS_World.hpp"
#include "glvm/ArchetypeECS/ArchetypeEntityManager.hpp"
#include "glvm/Archetypes/EnemyArchetype.hpp"
#include "glvm/Archetypes/PlayerArchetype.hpp"
#include "glvm/Archetypes/ProjectileArchetype.hpp"
#include "glvm/Components/ActorComponent.hpp"
#include "glvm/Components/DamageComponent.hpp"
#include "glvm/Components/MaterialComponent.hpp"
#include "glvm/Components/ProjectileBundle.hpp"
#include "glvm/Texture.hpp"

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

            vec3 distance = playerTransformComponent->position
                - enemyTransformComponent->position;
            float cameraSpeed = 5.5f * deltaFrameTime;

            if (projectileCooldown > 0) {
                projectileCooldown -= cameraSpeed;
            }
            if (distance.Length() > enemyComponent->detectRadius
                && stateEnemyComponent->state == core::States::ATTACK) {
                float deltaLength =
                    distance.Length() - enemyComponent->detectRadius;
                vec3 enemyMove = distance * (deltaLength / distance.Length());

                enemyTransformComponent->position += enemyMove;
            }

            if (distance.Length() <= enemyComponent->detectRadius) {
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
                        enemyTransformComponent->position,
                        playerTransformComponent->position
                            - enemyTransformComponent->position,
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
                    projectileCooldown = 5.0;
                }

                stateEnemyComponent->state = core::States::ATTACK;
            }
        }
    }
}
} // namespace glvm::ecs
