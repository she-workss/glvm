#pragma once

#include <cstdint>

namespace glvm::ecs::components {
struct MeshHandle {
    uint32_t id;
};

struct mesh {
    MeshHandle handle;
    bool gltf = true;
};
} // namespace glvm::ecs::components
