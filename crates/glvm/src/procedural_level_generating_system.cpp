#include "glvm/procedural_level_generating_system.hpp"

#include "glvm/archetype_ecs/arch_ecs_utils.hpp"
#include "glvm/archetype_ecs/arch_ecs_world.hpp"
#include "glvm/archetype_ecs/archetype_entity_manager.hpp"
#include "glvm/archetype_ecs/archetype_interface.hpp"
#include "glvm/archetypes/level_chunk_archetype.hpp"
#include "glvm/common/common_functions.hpp"
#include "glvm/components/collider_component.hpp"
#include "glvm/components/transform_component.hpp"
#include "glvm/components/vertex_component.hpp"
#include "glvm/constants.hpp"
#include "glvm/engine.hpp"
#include "glvm/graphic_api/vulkan.hpp"

#include <cmath>
#include <cstdint>
#include <random>
#include <vector>

namespace glvm::core {
void ProceduralLevelGeneratingSystem::Update() {
    using namespace glvm;
    namespace cm = glvm::ecs::components;
    namespace arch = glvm::ecs::arch;
    core::Engine* GLVM = core::Engine::GetInstance();

    // New arch ECS.
    arch::ArchetypeEntityManager* archEntityManager =
        arch::ArchetypeEntityManager::getInstance();

    arch::world.searchCacheArchetypes(
        playerRequiredMask,
        &archView.cachedPlayerArch,
        cachedPlayerArchNumber
    );
    componentsView.playerTransforms =
        (ecs::components::transform*)archView.cachedPlayerArch
            ->components[arch::ComponentsIndices::TRANSFORM_COMPONENT];

    while (levelNubmer < 5) {
        std::vector<core::Vertex> nextLevel;
        std::vector<uint32_t> indices;
        std::vector<core::Vertex> transitionBridgeVertices;
        std::vector<uint32_t> transitionBridgeIndices;

        if (levelNubmer < 5) {
            std::random_device rd;
            std::mt19937 mersenne(rd());
            std::uniform_int_distribution<int> distCurrentLevel_y(1, 1);
            unsigned int levelHalfY = distCurrentLevel_y(mersenne);
            std::uniform_int_distribution<int> distCurrentLevel_x_z(8, 16);
            unsigned int levelHalfX = distCurrentLevel_x_z(mersenne);
            unsigned int levelHalfZ = distCurrentLevel_x_z(mersenne);

            // Need to move on half.
            constexpr float transitionBridgeHalfWidth = 0.5f;
            constexpr float transitionBridgeHalfHeight = 1.0f;
            // On first iteration we dont need to define where locate current
            // level depends on previousTransitionBridge.
            if (levelNubmer != 0) {
                generateLevel(
                    levelHalfX,
                    levelHalfY,
                    levelHalfZ,
                    transitionBridgeHalfWidth,
                    transitionBridgeHalfHeight
                );
            } else {
                // Set to first level maximum values.
                coordinateMaximumValuePerDirection.lowest_x =
                    currentLevelPosition[0] - levelHalfX;
                coordinateMaximumValuePerDirection.highest_x =
                    currentLevelPosition[0] + levelHalfX;
                coordinateMaximumValuePerDirection.lowest_y =
                    currentLevelPosition[1] - levelHalfY;
                coordinateMaximumValuePerDirection.highest_y =
                    currentLevelPosition[1] + levelHalfY;
                coordinateMaximumValuePerDirection.lowest_z =
                    currentLevelPosition[2] - levelHalfZ;
                coordinateMaximumValuePerDirection.highest_z =
                    currentLevelPosition[2] + levelHalfZ;
            }

            generateTransitionBridge(
                levelHalfX,
                levelHalfY,
                levelHalfZ,
                transitionBridgeHalfWidth,
                transitionBridgeHalfHeight
            );

            for (unsigned int i = 0; i < 36; ++i) {
                indices.push_back(boxIndicesForIndexBuffer[i]);
            }

            meshAxisLimitingValues.setToDefaultValues();

            makeCubeObjectVertices(
                {-1, -1, -1, -1},
                {1, 1, 1, 1},
                levelHalfX,
                levelHalfY,
                levelHalfZ,
                nextLevel
            );
            setMeshBounds(meshAxisLimitingValues);

            [[maybe_unused]] cm::MeshHandle gameLevelMeshHandle =
                GLVM->LoadMesh();
            uint64_t gameLevelChunkEntity =
                archEntityManager->createEntity();

            cachedLevelChunkArchNumber = 0;
            // Search and cache one time for LevelChunkArch.
            arch::world.searchCacheArchetypes(
                requiredMask,
                &archView.cachedLevelChunkArch,
                cachedLevelChunkArchNumber
            );

            arch::world.addEntityToArchetype(
                gameLevelChunkEntity,
                archView.cachedLevelChunkArch
            );
            arch::EntityLocation gameLevelChunkLocation =
                arch::world.entityLocations[arch::getId(gameLevelChunkEntity)];

            arch::LevelChunkArchetype* levelChunkArch =
                static_cast<arch::LevelChunkArchetype*>(
                    gameLevelChunkLocation.arch
                );
            const uint32_t gameLevelChunkIndex = gameLevelChunkLocation.index;
            ecs::TextureHandle gameLevelTexture = textureHandlers[2];
            if (levelNubmer == 0) {
                // Set up current level position to player position.
                componentsView.playerTransforms->position = Vector<float, 3>(
                    currentLevelPosition[0],
                    componentsView.playerTransforms->position[1],
                    currentLevelPosition[2]
                );
            }
            levelChunkArch->transforms[gameLevelChunkIndex] = {
                .position = currentLevelPosition,
                .scale = 1.0f
            };
            levelChunkArch->materials[gameLevelChunkIndex] = {
                .diffuseTextureID_ = gameLevelTexture,
                .specularTextureID_ = gameLevelTexture,
                .ambient = {0.05f, 0.05f, 0.0f},
                .shininess = 128.0f * 0.078125f
            };
            levelChunkArch->meshes[gameLevelChunkIndex].handle =
                gameLevelMeshHandle;

            for (unsigned int i = 0; i < 36; ++i) {
                transitionBridgeIndices.push_back(boxIndicesForIndexBuffer[i]);
            }

            meshAxisLimitingValues.setToDefaultValues();

            float half_x = 0.0f;
            float half_y = levelHalfY;
            float half_z = 0.0f;
            setHalfExtentsFromDirection(
                half_x,
                half_z,
                transitionBridgeHalfWidth,
                transitionBridgeHalfHeight,
                nextLevelTransitionDirection
            );
            makeCubeObjectVertices(
                {-1, -1, -1, -1},
                {1, 1, 1, 1},
                half_x,
                half_y,
                half_z,
                transitionBridgeVertices
            );
            setMeshBounds(meshAxisLimitingValues);

            [[maybe_unused]] cm::MeshHandle transitionBridgeMeshHandle =
                GLVM->LoadMesh();
            uint64_t transitionBridgeEntity =
                archEntityManager->createEntity();
            arch::world.addEntityToArchetype(
                transitionBridgeEntity,
                archView.cachedLevelChunkArch
            );

            arch::EntityLocation transitionBridgeLocation =
                arch::world.entityLocations[arch::getId(transitionBridgeEntity)];

            arch::LevelChunkArchetype* transitionBridgeArch =
                static_cast<arch::LevelChunkArchetype*>(
                    transitionBridgeLocation.arch
                );
            const uint32_t transitionBridgeIndex =
                transitionBridgeLocation.index;
            ecs::TextureHandle transitionBridgeTexture = textureHandlers[3];
            transitionBridgeArch->transforms[transitionBridgeIndex] = {
                .position = transitionBridgePosition,
                .scale = 1.0f
            };
            transitionBridgeArch->materials[transitionBridgeIndex] = {
                .diffuseTextureID_ = transitionBridgeTexture,
                .specularTextureID_ = transitionBridgeTexture,
                .ambient = {0.05f, 0.05f, 0.0f},
                .shininess = 128.0f * 0.078125f
            };
            transitionBridgeArch->meshes[transitionBridgeIndex].handle =
                transitionBridgeMeshHandle;

            ++levelNubmer;
        }
        levelGeneratedVertices.push_back(nextLevel);
        levelGeneratedIndices.push_back(indices);

        levelGeneratedVertices.push_back(transitionBridgeVertices);
        levelGeneratedIndices.push_back(transitionBridgeIndices);

        bredoFlag = true;
    }
}

void ProceduralLevelGeneratingSystem::setHalfExtentsFromDirection(
    float& halfX,
    float& halfZ,
    const float& transitionBridgeHalfWidth,
    const float& transitionBridgeHalfHeight,
    const float& nextLevelTransitionDirection
) {
    if (nextLevelTransitionDirection == 1
        || nextLevelTransitionDirection == 3) {
        halfX = transitionBridgeHalfWidth;
        halfZ = transitionBridgeHalfHeight;
    } else if (
        nextLevelTransitionDirection == 2 || nextLevelTransitionDirection == 4
    ) {
        halfX = transitionBridgeHalfHeight;
        halfZ = transitionBridgeHalfWidth;
    }
}

void ProceduralLevelGeneratingSystem::generateLevel(
    const unsigned int levelHalfX,
    const unsigned int levelHalfY,
    const unsigned int levelHalfZ,
    const float transitionBridgeHalfWidth,
    const float transitionBridgeHalfHeight
) {
    std::random_device rd;
    std::mt19937 mersenne(rd());
    unsigned int previousTransitionBridgeAnchorPoint = 0;
    bool validLevel = false;
    while (!validLevel) {
        switch (previousIterationTransitionBridgeDirection) {
            case 1: {
                std::uniform_int_distribution<int>
                    distPreviousTransitionBridgeAnchorPoint(
                        0,
                        levelHalfX * 2 - 1
                    );
                previousTransitionBridgeAnchorPoint =
                    distPreviousTransitionBridgeAnchorPoint(mersenne);
                currentLevelPosition[0] = transitionBridgePosition[0]
                    - levelHalfX + transitionBridgeHalfWidth
                    + previousTransitionBridgeAnchorPoint;
                currentLevelPosition[2] = transitionBridgePosition[2]
                    + levelHalfZ + transitionBridgeHalfHeight;
            } break;
            case 2: {
                std::uniform_int_distribution<int>
                    distPreviousTransitionBridgeAnchorPoint(
                        0,
                        levelHalfZ * 2 - 1
                    );
                previousTransitionBridgeAnchorPoint =
                    distPreviousTransitionBridgeAnchorPoint(mersenne);
                currentLevelPosition[2] = transitionBridgePosition[2]
                    - levelHalfZ + transitionBridgeHalfWidth
                    + previousTransitionBridgeAnchorPoint;
                currentLevelPosition[0] = transitionBridgePosition[0]
                    + levelHalfX + transitionBridgeHalfHeight;
            } break;
            case 3: {
                std::uniform_int_distribution<int>
                    distPreviousTransitionBridgeAnchorPoint(
                        0,
                        levelHalfX * 2 - 1
                    );
                previousTransitionBridgeAnchorPoint =
                    distPreviousTransitionBridgeAnchorPoint(mersenne);
                currentLevelPosition[0] = transitionBridgePosition[0]
                    - levelHalfX + transitionBridgeHalfWidth
                    + previousTransitionBridgeAnchorPoint;
                currentLevelPosition[2] = transitionBridgePosition[2]
                    - levelHalfZ - transitionBridgeHalfHeight;
            } break;
            case 4: {
                std::uniform_int_distribution<int>
                    distPreviousTransitionBridgeAnchorPoint(
                        0,
                        levelHalfZ * 2 - 1
                    );
                previousTransitionBridgeAnchorPoint =
                    distPreviousTransitionBridgeAnchorPoint(mersenne);
                currentLevelPosition[2] = transitionBridgePosition[2]
                    - levelHalfZ + transitionBridgeHalfWidth
                    + previousTransitionBridgeAnchorPoint;
                currentLevelPosition[0] = transitionBridgePosition[0]
                    - levelHalfX - transitionBridgeHalfHeight;
            } break;
        }

        if (checkCollisionIntersectionWithMaximumCoordinates(
                currentLevelPosition,
                levelHalfX,
                levelHalfY,
                levelHalfZ
            )) {
            std::cout << "LEVEL COLLITION DETECTED" << std::endl;
            previousIterationTransitionBridgeDirection =
                (4 + previousIterationTransitionBridgeDirection) % 4 + 1;
        } else {
            coordinateMaximumValuePerDirection
                .comparePerDirectionAndSetToMaximumValueByModule(
                    currentLevelPosition,
                    (float)levelHalfX,
                    (float)levelHalfY,
                    (float)levelHalfZ
                );
            validLevel = true;
        }
    }
    currentLevelPosition[1] = 0.0f;
}

void ProceduralLevelGeneratingSystem::generateTransitionBridge(
    const unsigned int levelHalfX,
    const unsigned int levelHalfY,
    const unsigned int levelHalfZ,
    const float transitionBridgeHalfWidth,
    const float transitionBridgeHalfHeight
) {
    std::random_device rd;
    std::mt19937 mersenne(rd());
    // 1 - north, 2 - east, 3 - south, 4 - west.
    std::uniform_int_distribution<int> distNextLevelTransitionDirection(1, 4);
    // Randomly chose direction in where next level will appeared.
    nextLevelTransitionDirection = distNextLevelTransitionDirection(mersenne);
    unsigned int transitionBridgeAnchorPoint = 0;
    float transitionBridgeOffset_x = 0.0f;
    float transitionBridgeOffset_z = 0.0f;
    bool validTransitionBridge = false;
    while (!validTransitionBridge) {
        // Choose up (1) or down (3) insert point direction.
        if (nextLevelTransitionDirection == 1
            || nextLevelTransitionDirection == 3) {
            // In what point we connect next transition bridge to current level.
            std::uniform_int_distribution<int> distTransitionBridgeAnchorPoint(
                0,
                levelHalfX * 2 - 1
            );
            transitionBridgeAnchorPoint =
                distTransitionBridgeAnchorPoint(mersenne);
            // Summarize most left position with random value of point where
            // transition bridge will be insert.
            transitionBridgeOffset_x =
                -(float)levelHalfX + (float)transitionBridgeAnchorPoint;
            if (nextLevelTransitionDirection == 1) {
                // Move to the bottom level edge.
                transitionBridgeOffset_z = levelHalfZ;
                transitionBridgePosition = {
                    currentLevelPosition[0] + transitionBridgeOffset_x
                        + transitionBridgeHalfWidth,
                    (float)levelHalfY,
                    currentLevelPosition[2] + transitionBridgeOffset_z
                        + transitionBridgeHalfHeight
                };
            } else {
                // Move to the upper level edge.
                transitionBridgeOffset_z = -(float)levelHalfZ;
                transitionBridgePosition = {
                    currentLevelPosition[0] + transitionBridgeOffset_x
                        + transitionBridgeHalfWidth,
                    (float)levelHalfY,
                    currentLevelPosition[2] + transitionBridgeOffset_z
                        - transitionBridgeHalfHeight
                };
            }
            // Choose left (2) or right (4) insert point direction.
        } else if (
            nextLevelTransitionDirection == 2
            || nextLevelTransitionDirection == 4
        ) {
            // In what point we connect next transition bridge to current level.
            std::uniform_int_distribution<int> distTransitionBridgeAnchorPoint(
                0,
                levelHalfZ * 2 - 1
            );
            transitionBridgeAnchorPoint =
                distTransitionBridgeAnchorPoint(mersenne);
            // Summarize forward most position with random value of point where
            // transition bridge will be insert.
            transitionBridgeOffset_z =
                -(float)levelHalfZ + (float)transitionBridgeAnchorPoint;
            if (nextLevelTransitionDirection == 2) {
                // Move to the right level edge.
                transitionBridgeOffset_x = levelHalfX;
                transitionBridgePosition = {
                    currentLevelPosition[0] + transitionBridgeOffset_x
                        + transitionBridgeHalfHeight,
                    (float)levelHalfY,
                    currentLevelPosition[2] + transitionBridgeOffset_z
                        + transitionBridgeHalfWidth
                };
            } else {
                // Move to the left level edge.
                transitionBridgeOffset_x = -(float)levelHalfX;
                transitionBridgePosition = {
                    currentLevelPosition[0] + transitionBridgeOffset_x
                        - transitionBridgeHalfHeight,
                    (float)levelHalfY,
                    currentLevelPosition[2] + transitionBridgeOffset_z
                        + transitionBridgeHalfWidth
                };
            }
        }

        float width = 0;
        float height = 0;
        // Chose transitionBridgeHalfWidth as X and transitionBridgeHalfHeight
        // as Z.
        setHalfExtentsFromDirection(
            width,
            height,
            transitionBridgeHalfWidth,
            transitionBridgeHalfHeight,
            nextLevelTransitionDirection
        );

        if (checkCollisionIntersectionWithMaximumCoordinates(
                transitionBridgePosition,
                width,
                levelHalfY,
                height
            )) {
            // Need to choose another direction if we got collided with level.
            nextLevelTransitionDirection =
                (4 + nextLevelTransitionDirection) % 4 + 1;
        } else {
            // Setting up bounds for all levels.
            coordinateMaximumValuePerDirection
                .comparePerDirectionAndSetToMaximumValueByModule(
                    transitionBridgePosition,
                    (float)width,
                    (float)levelHalfY,
                    (float)height
                );
            validTransitionBridge = true;
        }
    }
    transitionBridgePosition[1] = currentLevelPosition[1];
    previousIterationTransitionBridgeDirection = nextLevelTransitionDirection;
}

void ProceduralLevelGeneratingSystem::makeCubeObjectVertices(
    Vector<float, 4> joinIndices,
    Vector<float, 4> weights,
    float half_x,
    float half_y,
    float half_z,
    std::vector<core::Vertex>& destinationVerticesContainer
) {
    unsigned int cube_vertices = 8;
    for (unsigned int i = 0; i < cube_vertices; ++i) {
        SVertex vertex;

        switch (i) {
            case 0:
                vertex[0] = half_x;
                vertex[1] = half_y;
                vertex[2] = half_z;
                break;
            case 1:
                vertex[0] = -(float)half_x;
                vertex[1] = half_y;
                vertex[2] = half_z;
                break;
            case 2:
                vertex[0] = -(float)half_x;
                vertex[1] = -(float)half_y;
                vertex[2] = half_z;
                break;
            case 3:
                vertex[0] = half_x;
                vertex[1] = -(float)half_y;
                vertex[2] = half_z;
                break;
            case 4:
                vertex[0] = half_x;
                vertex[1] = half_y;
                vertex[2] = -(float)half_z;
                break;
            case 5:
                vertex[0] = -(float)half_x;
                vertex[1] = half_y;
                vertex[2] = -(float)half_z;
                break;
            case 6:
                vertex[0] = -(float)half_x;
                vertex[1] = -(float)half_y;
                vertex[2] = -(float)half_z;
                break;
            case 7:
                vertex[0] = half_x;
                vertex[1] = -(float)half_y;
                vertex[2] = -(float)half_z;
                break;
        }

        meshAxisLimitingValues.comparePerDirectionAndSetToMaximumValueByModule(
            vertex
        );

        SVertex normal;
        normal[0] = 0;
        normal[1] = 1;
        normal[2] = 0;
        SVertex texture;
        texture[0] = 0;
        texture[1] = 1;

        destinationVerticesContainer.push_back(
            {{vertex[0], vertex[1], vertex[2]},
             {normal[0], normal[1], normal[2]},
             {texture[0], texture[1]},
             {joinIndices[0], joinIndices[1], joinIndices[2], joinIndices[3]},
             {weights[0], weights[1], weights[2], weights[3]}}
        );
    }
}

bool ProceduralLevelGeneratingSystem::
    checkCollisionIntersectionWithMaximumCoordinates(
        Vector<float, 3> position,
        float half_x,
        float half_y,
        float half_z
    ) {
    if (position[0] + half_x > coordinateMaximumValuePerDirection.lowest_x
        && position[0] - half_x < coordinateMaximumValuePerDirection.highest_x
        && position[1] + half_y > coordinateMaximumValuePerDirection.lowest_y
        && position[1] - half_y < coordinateMaximumValuePerDirection.highest_y
        && position[2] + half_z > coordinateMaximumValuePerDirection.lowest_z
        && position[2] - half_z
            < coordinateMaximumValuePerDirection.highest_z) {
        return true;
    } else {
        return false;
    }
}
} // namespace glvm::core
