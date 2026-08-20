#pragma once

#include "glvm/ArchetypeECS/ArchECS_World.hpp"
#include "glvm/ArchetypeECS/ArchetypeInterface.hpp"
#include "glvm/ComponentManager.hpp"
#include "glvm/Components/TransformComponent.hpp"
#include "glvm/EntityManager.hpp"
#include "glvm/GraphicAPI/Vulkan.hpp"
#include "glvm/ISystem.hpp"

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
    vec3 currentLevelPosition = {5.0f, 0.0f, 15.0f};
    vec3 transitionBridgePosition = {0.0f, 0.0f, 0.0f};
    unsigned int nextLevelTransitionDirection = 0;
    unsigned int previousIterationTransitionBridgeDirection = 0;

    u32 cachedLevelChunkArchNumber = 0;
    u32 cachedPlayerArchNumber = 0;

    struct ProceduralLevelArchView {
        ecs::arch::Archetype* cachedLevelChunkArch = nullptr;
        ecs::arch::Archetype* cachedPlayerArch = nullptr;
    } archView;

    struct ComponentsView {
        ecs::components::transform* playerTransforms = nullptr;
    } componentsView;

    ecs::arch::componentMask playerRequiredMask =
        (1ull << ecs::arch::ComponentsIndices::PLAYER_TAG_COMPONENT);

    ecs::arch::componentMask requiredMask =
        (1ull << ecs::arch::ComponentsIndices::TRANSFORM_COMPONENT)
        | (1ull << ecs::arch::ComponentsIndices::MATERIAL_COMPONENT)
        | (1ull << ecs::arch::ComponentsIndices::MESH_COMPONENT)
        | (1ull << ecs::arch::ComponentsIndices::COLLIDER_COMPONENT)
        | (1ull << ecs::arch::ComponentsIndices::COLLIDER_FLAGS_COMPONENT)
        | (1ull << ecs::arch::ComponentsIndices::LEVEL_CHUNK_TAG_COMPONENT);

    core::vector<ecs::components::MeshHandle> meshHandlers;
    core::vector<ecs::TextureHandle> textureHandlers;

    std::vector<core::vector<core::Vertex>> levelGeneratedVertices;
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
        vec4 joinIndices,
        vec4 weights,
        float half_x,
        float half_y,
        float half_z,
        core::vector<core::Vertex>& destinationVerticesContainer
    );
    bool checkCollisionIntersectionWithMaximumCoordinates(
        vec3 position,
        float half_x,
        float half_y,
        float half_z
    );
};
} // namespace glvm::core
