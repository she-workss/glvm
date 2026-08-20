#pragma once

#include "glvm/archetype_ecs/arch_ecs_types.hpp"
#include "glvm/archetype_ecs/archetype_interface.hpp"
#include "glvm/common/common_functions.hpp"
#include "glvm/components/damage_component.hpp"
#include "glvm/components/enemy_component.hpp"
#include "glvm/components/state_component.hpp"
#include "glvm/components/transform_component.hpp"
#include "glvm/entity_manager.hpp"
#include "glvm/i_sound_engine.hpp"
#include "glvm/i_system.hpp"

#include <vector>

namespace glvm::ecs {
class EnemySystem: public ISystem {
public:
    uint32_t playerArchetypesNumber = 0;
    uint32_t enemyArchetypesNumber = 0;
    uint32_t projectileArchetypesNumber = 0;

    struct ArchView {
        arch::Archetype* playerCachedArchetype = nullptr;
        arch::Archetype* enemyCachedArchetype = nullptr;
        arch::Archetype* projectileArchetype = nullptr;
    } archView;

    struct ComponentsView {
        ecs::components::transform* playerTransforms = nullptr;

        ecs::components::transform* enemyTransforms = nullptr;
        ecs::components::state* enemyStates = nullptr;
        ecs::components::enemy* enemies = nullptr;
    } componentsView;

    uint64_t playerRequiredMask =
        (1ull << ecs::arch::ComponentsIndices::PLAYER_TAG_COMPONENT);

    uint64_t enemyRequiredMask =
        (1ul << arch::ComponentsIndices::TRANSFORM_COMPONENT)
        | (1ul << arch::ComponentsIndices::STATE_COMPONENT)
        | (1ul << arch::ComponentsIndices::ENEMY_COMPONENT);

    uint64_t projectileRequiredMask =
        (1ull << ecs::arch::ComponentsIndices::PROJECTILE_TAG_COMPONENT);

    void Update() override;
    core::Sound::ISoundEngine* soundEngine;
    std::vector<ecs::TextureHandle> textureHandlers;
    std::vector<ecs::components::MeshHandle> meshHandlers;
    float projectileCooldown = 5.0f;
    float deltaFrameTime;
};
} // namespace glvm::ecs
