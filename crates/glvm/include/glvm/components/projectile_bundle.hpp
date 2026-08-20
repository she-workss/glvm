#pragma once

#include "glvm/components/damage_component.hpp"
#include "glvm/components/material_component.hpp"
#include "glvm/components/projectile_component.hpp"

namespace glvm::ecs::arch {
struct ProjectileBundle {
    components::projectile projectile;
    components::damage damage;
    components::material material;
};
}; // namespace glvm::ecs::arch
