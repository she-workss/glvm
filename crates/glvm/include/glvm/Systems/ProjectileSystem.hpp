// This file is part of Game Loop Versatile Modules (GLVM)
// Copyright © 2024 Maksim Manokhin a.k.a. Yuriorkis_Scream. Contacts:
// <fellfrostqtw@gmail.com> Author: Maksim Manokhin a.k.a. Yuriorkis_Scream
// License: http://opensource.org/licenses/MIT

#ifndef PROJECTILE_SYSTEM
#define PROJECTILE_SYSTEM

#include "glvm/ArchetypeECS/ArchECS_Utils.hpp"
#include "glvm/Archetypes/EnemyArchetype.hpp"
#include "glvm/Archetypes/PlayerArchetype.hpp"
#include "glvm/Common/CommonFunctions.hpp"
#include "glvm/ComponentManager.hpp"
#include "glvm/Components/AttackComponent.hpp"
#include "glvm/Components/ColliderComponent.hpp"
#include "glvm/Components/ColliderFlagsComponent.hpp"
#include "glvm/Components/HealthComponent.hpp"
#include "glvm/Components/MaterialComponent.hpp"
#include "glvm/Components/MoveComponent.hpp"
#include "glvm/Components/ProjectileBundle.hpp"
#include "glvm/Components/ProjectileComponent.hpp"
#include "glvm/Components/TransformComponent.hpp"
#include "glvm/Components/ViewComponent.hpp"
#include "glvm/EntityManager.hpp"
#include "glvm/EventsStack.hpp"
#include "glvm/Globals.hpp"
#include "glvm/ISoundEngine.hpp"
#include "glvm/ISystem.hpp"
#include "glvm/TextureManager.hpp"
#include "glvm/Vector.hpp"

#include <cstdint>

namespace GLVM::ecs {
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
    core::vector<ecs::TextureHandle> textureHandlers;
    core::vector<ecs::components::MeshHandle> meshHandlers;
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

    arch::componentMask playerRequiredMask =
        (1ull << ecs::arch::ComponentsIndices::PLAYER_TAG_COMPONENT);

    arch::componentMask projectileRequiredMask =
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
    // void CalculateProjectile(const vec3& projectilePosition,
    // 						 const vec3& projectileForward,
    // 						 const ecs::components::MeshHandle& meshHandle,
    // 						 const components::material& material,
    // 						 const components::damage& damage);
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

} // namespace GLVM::ecs

#endif
