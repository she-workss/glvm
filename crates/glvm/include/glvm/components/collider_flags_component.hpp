#pragma once

namespace glvm::ecs::components {
struct colliderFlags {
    // 0001 = wallCollision; 0010 = groundCollision; 0100 = roofCollision; 1000
    // = itemDrag.
    int flags : 4;
};
}; // namespace glvm::ecs::components
