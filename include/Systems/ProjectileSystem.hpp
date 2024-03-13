// This file is part of Game Loop Versatile Modules (GLVM)
// Copyright © 2024 Maksim Manokhin a.k.a. Yuriorkis_Scream. Contacts:
// <fellfrostqtw@gmail.com> Author: Maksim Manokhin a.k.a. Yuriorkis_Scream
// License: http://opensource.org/licenses/MIT

#ifndef PROJECTILE_SYSTEM
#define PROJECTILE_SYSTEM

#include "ComponentManager.hpp"
#include "EntityManager.hpp"
#include "EventsStack.hpp"
#include "Globals.hpp"
#include "ISoundEngine.hpp"
#include "ISystem.hpp"
#include "TextureManager.hpp"
#include "Vector.hpp"
#include "components/ColliderComponent.hpp"
#include "components/MaterialComponent.hpp"
#include "components/MoveComponent.hpp"
#include "components/ProjectileComponent.hpp"
#include "components/TransformComponent.hpp"
#include "components/ViewComponent.hpp"

namespace GLVM::ecs {
class CProjectileSystem : public ISystem {
public:
    float fYaw = -90.0f;
    float fPitch = 0.0f;
    float fLast_X = 1920.0f / 2.0f;
    float fLast_Y = 1080.0f / 2.0f;
    bool bFirst_Mouse = true;
    core::CStack &inputStack;
    core::vector<ecs::TextureHandle> textureHandlers;
    core::vector<ecs::components::MeshHandle> meshHandlers;
    core::Sound::ISoundEngine *soundEngine;
    float projectileCooldown = 2.0f;
    float deltaFrameTime;

    CProjectileSystem(core::CStack &inputStack);
    void Update() override;
    void CalculateProjectile(ecs::ComponentManager *componentManager,
                             unsigned int entityRefMove,
                             components::beholder &beholder);

    Vector<float, 3> GetDirectionVector(components::beholder &beholder);
};
} // namespace GLVM::ecs

#endif
