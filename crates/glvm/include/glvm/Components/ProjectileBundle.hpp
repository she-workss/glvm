#ifndef PROJECTILE_BUNDLE_HPP
#define PROJECTILE_BUNDLE_HPP

#include "glvm/Components/DamageComponent.hpp"
#include "glvm/Components/MaterialComponent.hpp"
#include "glvm/Components/ProjectileComponent.hpp"

namespace GLVM::ecs::arch {
struct ProjectileBundle {
    components::projectile projectile;
    components::damage damage;
    components::material material;
};
}; // namespace GLVM::ecs::arch

#endif
