#pragma once

namespace glvm::ecs::components {
class projectile {
public:
    unsigned int owner;
    bool bCollision_Status_ = false;
    float fDamage_;
    float fSpeed_;
    float fFlying_Range_;
    float damage;
};
} // namespace glvm::ecs::components
