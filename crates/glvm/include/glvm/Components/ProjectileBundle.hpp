#pragma once

#include "glvm/Components/DamageComponent.hpp"
#include "glvm/Components/MaterialComponent.hpp"
#include "glvm/Components/ProjectileComponent.hpp"

namespace glvm::ecs::arch {
struct ProjectileBundle {
    components::projectile projectile;
    components::damage damage;
    components::material material;
};
}; // namespace glvm::ecs::arch
