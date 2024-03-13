// This file is part of Game Loop Versatile Modules (GLVM)
// Copyright © 2024 Maksim Manokhin a.k.a. Yuriorkis_Scream. Contacts:
// <fellfrostqtw@gmail.com> Author: Maksim Manokhin a.k.a. Yuriorkis_Scream
// License: http://opensource.org/licenses/MIT
#pragma once

#include "component_manager.hpp"
#include "components/collider_component.hpp"
#include "components/material_component.hpp"
#include "components/move_component.hpp"
#include "components/projectile_component.hpp"
#include "components/transform_component.hpp"
#include "components/view_component.hpp"
#include "entity_manager.hpp"
#include "events_stack.hpp"
#include "globals.hpp"
#include "i_sound_engine.hpp"
#include "i_system.hpp"
#include "texture_manager.hpp"
#include "vector.hpp"

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
