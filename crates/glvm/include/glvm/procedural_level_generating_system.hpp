#pragma once

#include "glvm/archetype_ecs/arch_ecs_world.hpp"
#include "glvm/archetype_ecs/archetype_interface.hpp"
#include "glvm/component_manager.hpp"
#include "glvm/components/transform_component.hpp"
#include "glvm/entity_manager.hpp"
#include "glvm/graphic_api/vulkan.hpp"
#include "glvm/i_system.hpp"

#include <cmath>
#include <cstdint>
#include <map>
#include <random>

namespace glvm::core {
class ProceduralLevelGeneratingSystem: public ecs::ISystem {
public:
    unsigned int levelNubmer = 0;
    bool bredoFlag = false;
    unsigned int previous_half_x_rand = 0;
    unsigned int previous_half_z_rand = 0;
    Vector<float, 3> currentLevelPosition = {5.0f, 0.0f, 15.0f};
    Vector<float, 3> transitionBridgePosition = {0.0f, 0.0f, 0.0f};
    unsigned int nextLevelTransitionDirection = 0;
    unsigned int previousIterationTransitionBridgeDirection = 0;

    uint32_t cachedLevelChunkArchNumber = 0;
    uint32_t cachedPlayerArchNumber = 0;

    struct ProceduralLevelArchView {
        ecs::arch::Archetype* cachedLevelChunkArch = nullptr;
        ecs::arch::Archetype* cachedPlayerArch = nullptr;
    } archView;

    struct ComponentsView {
        ecs::components::transform* playerTransforms = nullptr;
    } componentsView;

    uint64_t playerRequiredMask =
        (1ull << ecs::arch::ComponentsIndices::PLAYER_TAG_COMPONENT);

    uint64_t requiredMask =
        (1ull << ecs::arch::ComponentsIndices::TRANSFORM_COMPONENT)
        | (1ull << ecs::arch::ComponentsIndices::MATERIAL_COMPONENT)
        | (1ull << ecs::arch::ComponentsIndices::MESH_COMPONENT)
        | (1ull << ecs::arch::ComponentsIndices::COLLIDER_COMPONENT)
        | (1ull << ecs::arch::ComponentsIndices::COLLIDER_FLAGS_COMPONENT)
        | (1ull << ecs::arch::ComponentsIndices::LEVEL_CHUNK_TAG_COMPONENT);

    std::vector<ecs::components::MeshHandle> meshHandlers;
    std::vector<ecs::TextureHandle> textureHandlers;

    std::vector<std::vector<core::Vertex>> levelGeneratedVertices;
    // Wavefront .obj indices.
    std::vector<std::vector<uint32_t>> levelGeneratedIndices;
    // Keep axis limiting values for every axis per mesh in current iteration
    // while initializing Wavefront .obj and GLTF.
    MeshAxisLimitingValues meshAxisLimitingValues;
    // Contains maximum coordinate value in every direction for all generated
    // levels.
    MeshAxisLimitingValues coordinateMaximumValuePerDirection;

    void Update();
    void setHalfExtentsFromDirection(
        float& halfX,
        float& halfZ,
        const float& transitionBridgeHalfWidth,
        const float& transitionBridgeHalfHeight,
        const float& nextLevelTransitionDirection
    );
    void generateLevel(
        const unsigned int levelHalfX,
        const unsigned int levelHalfY,
        const unsigned int levelHalfZ,
        const float transitionBridgeHalfWidth,
        const float transitionBridgeHalfHeight
    );
    void generateTransitionBridge(
        const unsigned int levelHalfX,
        const unsigned int levelHalfY,
        const unsigned int levelHalfZ,
        const float transitionBridgeHalfWidth,
        const float transitionBridgeHalfHeight
    );
    void makeCubeObjectVertices(
        Vector<float, 4> joinIndices,
        Vector<float, 4> weights,
        float half_x,
        float half_y,
        float half_z,
        std::vector<core::Vertex>& destinationVerticesContainer
    );
    bool checkCollisionIntersectionWithMaximumCoordinates(
        Vector<float, 3> position,
        float half_x,
        float half_y,
        float half_z
    );
};
} // namespace glvm::core
