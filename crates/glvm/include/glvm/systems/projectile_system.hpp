#pragma once

#include "glvm/archetype_ecs/arch_ecs_utils.hpp"
#include "glvm/archetypes/enemy_archetype.hpp"
#include "glvm/archetypes/player_archetype.hpp"
#include "glvm/common/common_functions.hpp"
#include "glvm/component_manager.hpp"
#include "glvm/components/attack_component.hpp"
#include "glvm/components/collider_component.hpp"
#include "glvm/components/collider_flags_component.hpp"
#include "glvm/components/health_component.hpp"
#include "glvm/components/material_component.hpp"
#include "glvm/components/move_component.hpp"
#include "glvm/components/projectile_bundle.hpp"
#include "glvm/components/projectile_component.hpp"
#include "glvm/components/transform_component.hpp"
#include "glvm/components/view_component.hpp"
#include "glvm/entity_manager.hpp"
#include "glvm/events_stack.hpp"
#include "glvm/globals.hpp"
#include "glvm/i_sound_engine.hpp"
#include "glvm/i_system.hpp"
#include "glvm/texture_manager.hpp"

#include <cstdint>
#include <vector>

namespace glvm::ecs {
template<typename T>
concept UnitOrEnemy = std::is_same_v<T, arch::PlayerArchetype>
    || std::is_same_v<T, arch::EnemyArchetype>;

template<typename T>
concept HasAttack = requires(T* t) {
    { t->attacks };
};

class CProjectileSystem: public ISystem {
public:
    float fYaw = -90.0f;
    float fPitch = 0.0f;
    bool bFirst_Mouse = true;
    core::CStack& inputStack;
    std::vector<ecs::TextureHandle> textureHandlers;
    std::vector<ecs::components::MeshHandle> meshHandlers;
    core::Sound::ISoundEngine* soundEngine;
    float projectileCooldown = 2.0f;
    float deltaFrameTime;
    bool isInventoryOpened;

    uint32_t playerArchetypesNumber = 0;
    uint32_t projectileArchetypesNumber = 0;

    struct ArchView {
        arch::Archetype* playerCachedArchetype = nullptr;
        arch::Archetype* projectileArchetype = nullptr;
    } archView;

    struct ComponentsView {
        ecs::components::transform* playerTransforms = nullptr;
        ecs::components::beholder* playerViews = nullptr;

        ecs::components::transform* projectileTransforms = nullptr;
        ecs::components::colliderFlags* projectileColliderFlags = nullptr;
        ecs::components::collider* projectileColliders = nullptr;
        arch::ProjectileBundle* projectileBundles = nullptr;
        ecs::components::health* projectileHealth = nullptr;
        ecs::components::attack* projectileAttacks = nullptr;
    } componentsView;

    uint64_t playerRequiredMask =
        (1ull << ecs::arch::ComponentsIndices::PLAYER_TAG_COMPONENT);

    uint64_t projectileRequiredMask =
        (1ull << ecs::arch::ComponentsIndices::PROJECTILE_TAG_COMPONENT);

    CProjectileSystem(core::CStack& inputStack);
    void Update() override;
    template<typename T>
        requires UnitOrEnemy<T> && HasAttack<T>
    static void markAsAttacked(
        T* arch,
        components::damage* projectileDamage,
        uint32_t entityIndex
    );
};

template<typename T>
    requires UnitOrEnemy<T> && HasAttack<T>
void CProjectileSystem::markAsAttacked(
    T* arch,
    components::damage* projectileDamage,
    uint32_t enitityIndex
) {
    arch->attacks[enitityIndex].damage = projectileDamage->maximumDamage;
}

} // namespace glvm::ecs
