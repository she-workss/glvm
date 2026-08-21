#include "glvm/glvm.hpp"

#include "imgui.h"
#include "imgui_impl_vulkan.h"

#include <algorithm>
#include <cassert>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <fstream>
#include <iostream>
#include <iterator>
#include <limits>
#include <mutex>
#include <ostream>
#include <pthread.h>
#include <random>
#include <stdexcept>
#include <string>
#include <sys/types.h>
#include <thread>
#include <unistd.h>
#include <vector>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_wayland.h>
#ifdef _WIN32
#include "imgui_impl_win32.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef NOGDI
#define NOGDI
#endif
// clang-format off
#include <windows.h>
#include <mmsystem.h>
// clang-format on
#endif // _WIN32
#ifdef __linux__
#include <wayland-client-core.h>
#endif // __linux__

#ifdef _WIN32
constexpr auto VK_W = 0x57;
constexpr auto VK_S = 0x53;
constexpr auto VK_A = 0x41;
constexpr auto VK_D = 0x44;
constexpr auto VK_I = 0x49;
#endif

namespace glvm {
auto matches_required_mask(
    const uint64_t archetype_mask,
    const uint64_t& system_mask
) -> bool {
    return (archetype_mask & system_mask) == system_mask;
}

auto makeEntity(uint32_t id_, uint32_t generation_) -> uint64_t {
    return ((uint64_t)generation_ << ENTITY_ID_BITS) | id_;
}

auto getId(uint64_t entity_) -> uint32_t {
    return entity_ & entityBitsMask;
}

auto getGen(uint64_t entity_) -> uint32_t {
    return entity_ >> ENTITY_ID_BITS;
}
}; // namespace glvm

namespace glvm {
World world = {};

World::World() {
    assert(
        spatialGrid.width > 0 && spatialGrid.height > 0 && spatialGrid.depth > 0
    );

    const float chunkSize = spatialGrid.grid[0][0][0].size;
    const float halfWorldWidth = spatialGrid.width * chunkSize * 0.5f;
    const float halfWorldHeight = spatialGrid.height * chunkSize * 0.5f;
    const float halfWorldDepth = spatialGrid.depth * chunkSize * 0.5f;
    const float halfChunkSize = chunkSize * 0.5f;
    const Vector<float, 3> pivot = Vector<float, 3>(
        -halfWorldWidth + halfChunkSize,
        -halfWorldHeight + halfChunkSize,
        -halfWorldDepth + halfChunkSize
    );
    for (uint32_t i0 = 0; i0 < spatialGrid.depth; ++i0) {
        for (uint32_t i1 = 0; i1 < spatialGrid.height; ++i1) {
            for (uint32_t i2 = 0; i2 < spatialGrid.width; ++i2) {
                spatialGrid.grid[i0][i1][i2].position = Vector<float, 3>(
                                                            i2 * chunkSize,
                                                            i1 * chunkSize,
                                                            i0 * chunkSize
                                                        )
                    + pivot;
            }
        }
    }
}

World::~World() {
    for (unsigned int i = 0; i < archetypes.size(); ++i) {
        delete archetypes[i];
        archetypes[i] = nullptr;
    }
}

void World::addEntityToArchetype(uint64_t entity_, Archetype* arch) {
    uint32_t id_ = getId(entity_);

    if (id_ >= entityLocations.size()) {
        entityLocations.resize(id_ + 1);
    }

    EntityLocation& location = entityLocations[id_];

    if (location.arch != nullptr) {
        assert(false && "unsigned int already assigned to archetype");
    }

    uint32_t index = arch->addEntity(entity_);

    location.arch = arch;
    location.index = index;
}

void World::removeEntity(uint64_t entity_) {
    uint32_t id_ = getId(entity_);
    EntityLocation& location = entityLocations[id_];
    // Remove entity from spatial grid cells it occupies, otherwise stale
    // references crash collision/physics on later frames.
    if (location.gridCellCounter > 0) {
        for (uint8_t i = 0; i < location.gridCellCounter; ++i) {
            uint32_t z = location.gridCellIndicies[i][0];
            uint32_t y = location.gridCellIndicies[i][1];
            uint32_t x = location.gridCellIndicies[i][2];
            std::vector<uint32_t>& chunkEntities =
                spatialGrid.grid[z][y][x].entities;
            for (uint32_t k = 0; k < chunkEntities.size(); ++k) {
                if (chunkEntities[k] == entity_) {
                    chunkEntities.erase(chunkEntities.begin() + k);
                    break;
                }
            }
        }
        location.gridCellCounter = 0;
    }

    Archetype* arch = location.arch;
    uint32_t index = location.index;

    uint64_t moved = arch->removeEntity(index);

    if (moved != entity_) {
        uint32_t movedId = getId(moved);
        entityLocations[movedId].index = index;
        entityLocations[movedId].arch = arch;
    }
    std::cout << "remove entity with id: " << id_ << std::endl;
    location.arch = nullptr;
}

void World::searchCacheArchetypes(
    uint64_t requiredMask,
    Archetype* cachedArchetypes[],
    uint32_t& cachedArchetypesNumber
) {
    for (uint32_t i = 0; i < world.archetypes.size(); ++i) {
        Archetype* arch = world.archetypes[i];

        if ((arch->mask & requiredMask) == requiredMask) {
            cachedArchetypes[cachedArchetypesNumber] = arch;
            ++cachedArchetypesNumber;
        }
    }
}
}; // namespace glvm

namespace glvm {
ArchetypeEntityManager* ArchetypeEntityManager::pInstance_ = nullptr;
std::mutex ArchetypeEntityManager::Mutex_;

ArchetypeEntityManager::ArchetypeEntityManager() {}

ArchetypeEntityManager::~ArchetypeEntityManager() {}

ArchetypeEntityManager* ArchetypeEntityManager::get_instance() {
    std::lock_guard<std::mutex> lock(Mutex_);
    if (pInstance_ == nullptr) {
        pInstance_ = new ArchetypeEntityManager();
    }
    return pInstance_;
}

[[nodiscard]] uint64_t ArchetypeEntityManager::createEntity() {
    uint32_t newId = 0;
    // Check out wether or not free ID in removed entities registry.
    if (!freeList.empty()) {
        newId = freeList.back();
        freeList.pop_back();
    } else {
        newId = nextId++;
        generations.push_back(1);
    }

    return makeEntity(newId, generations[newId]);
}

void ArchetypeEntityManager::removeEntity(uint64_t entity_) {
    uint32_t id_ = getId(entity_);

    if (!isAlive(entity_)) {
        return;
    }

    generations[id_]++;
    freeList.push_back(id_);
}

bool ArchetypeEntityManager::isAlive(uint64_t entity_) const {
    uint32_t id_ = getId(entity_);
    return id_ < generations.size() && generations[id_] == getGen(entity_);
}
}; // namespace glvm

namespace glvm {
uint32_t Archetype::addEntity(uint64_t entity_) {
    uint32_t index = entityCount++;
    assert(index < CAPACITY);
    entities[index] = entity_;

    return index;
}

// Swap-remove.
uint64_t Archetype::removeEntity(uint32_t index) {
    uint32_t last = entityCount - 1;

    for (uint32_t i = 0; i < componentCount; ++i) {
        const uint32_t componentId = componentIds[i];

        switch (componentId) {
            case ComponentsIndices::TransformComponent:
                static_cast<transform*>(components[componentId])[index] =
                    static_cast<transform*>(components[componentId])[last];
                break;
            case ComponentsIndices::RigidBodyComponent:
                static_cast<RigidBody*>(components[componentId])[index] =
                    static_cast<RigidBody*>(components[componentId])[last];
                break;
            case ComponentsIndices::MeshComponent:
                static_cast<mesh*>(components[componentId])[index] =
                    static_cast<mesh*>(components[componentId])[last];
                break;
            case ComponentsIndices::FontComponent:
                static_cast<font*>(components[componentId])[index] =
                    static_cast<font*>(components[componentId])[last];
                break;
            case ComponentsIndices::ColliderComponent:
                static_cast<collider*>(components[componentId])[index] =
                    static_cast<collider*>(components[componentId])[last];
                break;
            case ComponentsIndices::ColliderFlagsComponent:
                static_cast<colliderFlags*>(components[componentId])[index] =
                    static_cast<colliderFlags*>(components[componentId])[last];
                break;
            case ComponentsIndices::MaterialComponent:
                static_cast<material*>(components[componentId])[index] =
                    static_cast<material*>(components[componentId])[last];
                break;
            case ComponentsIndices::ViewComponent:
                static_cast<beholder*>(components[componentId])[index] =
                    static_cast<beholder*>(components[componentId])[last];
                break;
            case ComponentsIndices::HealthComponent:
                static_cast<health*>(components[componentId])[index] =
                    static_cast<health*>(components[componentId])[last];
                break;
            case ComponentsIndices::AnimationComponent:
                static_cast<animation*>(components[componentId])[index] =
                    static_cast<animation*>(components[componentId])[last];
                break;
            case ComponentsIndices::StateComponent:
                static_cast<state*>(components[componentId])[index] =
                    static_cast<state*>(components[componentId])[last];
                break;
            case ComponentsIndices::EnemyComponent:
                static_cast<enemy*>(components[componentId])[index] =
                    static_cast<enemy*>(components[componentId])[last];
                break;
            case ComponentsIndices::DamageComponent:
                static_cast<damage*>(components[componentId])[index] =
                    static_cast<damage*>(components[componentId])[last];
                break;
            case ComponentsIndices::AttackComponent:
                static_cast<attack*>(components[componentId])[index] =
                    static_cast<attack*>(components[componentId])[last];
                break;
            case ComponentsIndices::InventoryComponent:
                static_cast<inventory*>(components[componentId])[index] =
                    static_cast<inventory*>(components[componentId])[last];
                break;
            case ComponentsIndices::DirectionalLightComponent:
                static_cast<directionalLight*>(components[componentId])[index] =
                    static_cast<directionalLight*>(
                        components[componentId]
                    )[last];
                break;
            case ComponentsIndices::SpotLightComponent:
                static_cast<spotLight*>(components[componentId])[index] =
                    static_cast<spotLight*>(components[componentId])[last];
                break;
            case ComponentsIndices::PointLightComponent:
                static_cast<pointLight*>(components[componentId])[index] =
                    static_cast<pointLight*>(components[componentId])[last];
                break;
            case ComponentsIndices::ItemComponent:
                static_cast<item*>(components[componentId])[index] =
                    static_cast<item*>(components[componentId])[last];
                break;
            case ComponentsIndices::MoveComponent:
                static_cast<move*>(components[componentId])[index] =
                    static_cast<move*>(components[componentId])[last];
                break;
            case ComponentsIndices::ProjectileBundleComponent:
                static_cast<ProjectileBundle*>(components[componentId])[index] =
                    static_cast<ProjectileBundle*>(
                        components[componentId]
                    )[last];
                break;
            case ComponentsIndices::LevelChunkTagComponent:
                static_cast<levelChunkTagComponent*>(
                    components[componentId]
                )[index] =
                    static_cast<levelChunkTagComponent*>(
                        components[componentId]
                    )[last];
                break;
            case ComponentsIndices::ProjectileTagComponent:
                static_cast<projectileTagComponent*>(
                    components[componentId]
                )[index] =
                    static_cast<projectileTagComponent*>(
                        components[componentId]
                    )[last];
                break;
            case ComponentsIndices::PlayerTagComponent:
                static_cast<playerTagComponent*>(
                    components[componentId]
                )[index] =
                    static_cast<playerTagComponent*>(
                        components[componentId]
                    )[last];
                break;
        }
    }

    uint64_t moved = entities[last];
    entities[index] = moved;
    --entityCount;

    return moved;
}
}; // namespace glvm

namespace glvm {
bool BoxCollider(
    const Vector<float, 3> backtrackingPos,
    const Vector<float, 3> comparedPos,
    const float backtrackingScale,
    const float comparedScale,
    const MeshAxisMaxAbsoluteValues& backtrackingMeshAxisMaxAbsoluteValues,
    const MeshAxisMaxAbsoluteValues& comparedMeshAxisMaxAbsoluteValues
) {
    return backtrackingPos[0]
            + backtrackingMeshAxisMaxAbsoluteValues.origin_offset_x
                * backtrackingScale
            + (backtrackingMeshAxisMaxAbsoluteValues.absolute_x
               * backtrackingScale)
        > comparedPos[0]
            + comparedMeshAxisMaxAbsoluteValues.origin_offset_x * comparedScale
            - (comparedMeshAxisMaxAbsoluteValues.absolute_x * comparedScale)
        && backtrackingPos[0]
            + backtrackingMeshAxisMaxAbsoluteValues.origin_offset_x
                * backtrackingScale
            - (backtrackingMeshAxisMaxAbsoluteValues.absolute_x
               * backtrackingScale)
        < comparedPos[0]
            + comparedMeshAxisMaxAbsoluteValues.origin_offset_x * comparedScale
            + (comparedMeshAxisMaxAbsoluteValues.absolute_x * comparedScale)
        && backtrackingPos[1]
            + backtrackingMeshAxisMaxAbsoluteValues.origin_offset_y
                * backtrackingScale
            + (backtrackingMeshAxisMaxAbsoluteValues.absolute_y
               * backtrackingScale)
        > comparedPos[1]
            + comparedMeshAxisMaxAbsoluteValues.origin_offset_y * comparedScale
            - (comparedMeshAxisMaxAbsoluteValues.absolute_y * comparedScale)
        && backtrackingPos[1]
            + backtrackingMeshAxisMaxAbsoluteValues.origin_offset_y
                * backtrackingScale
            - (backtrackingMeshAxisMaxAbsoluteValues.absolute_y
               * backtrackingScale)
        < comparedPos[1]
            + comparedMeshAxisMaxAbsoluteValues.origin_offset_y * comparedScale
            + (comparedMeshAxisMaxAbsoluteValues.absolute_y * comparedScale)
        && backtrackingPos[2]
            + backtrackingMeshAxisMaxAbsoluteValues.origin_offset_z
                * backtrackingScale
            + (backtrackingMeshAxisMaxAbsoluteValues.absolute_z
               * backtrackingScale)
        > comparedPos[2]
            + comparedMeshAxisMaxAbsoluteValues.origin_offset_z * comparedScale
            - (comparedMeshAxisMaxAbsoluteValues.absolute_z * comparedScale)
        && backtrackingPos[2]
            + backtrackingMeshAxisMaxAbsoluteValues.origin_offset_z
                * backtrackingScale
            - (backtrackingMeshAxisMaxAbsoluteValues.absolute_z
               * backtrackingScale)
        < comparedPos[2]
            + comparedMeshAxisMaxAbsoluteValues.origin_offset_z * comparedScale
            + (comparedMeshAxisMaxAbsoluteValues.absolute_z * comparedScale);
}

std::vector<Vector<float, 3>> computeBoxCornerBoundPoints(
    const MeshAxisMaxAbsoluteValues entityChunkBounds,
    Vector<float, 3> entityPosition,
    const float scale
) {
    const float halfWidht = entityChunkBounds.absolute_x * scale;
    const float halfHeight = entityChunkBounds.absolute_y * scale;
    const float halfDepth = entityChunkBounds.absolute_z * scale;
    const Vector<float, 3> centerOffset = {
        entityChunkBounds.origin_offset_x * scale,
        entityChunkBounds.origin_offset_y * scale,
        entityChunkBounds.origin_offset_z * scale
    };
    std::vector<Vector<float, 3>> result;
    // Left bottom back.
    result.push_back(
        entityPosition + centerOffset
        + Vector<float, 3>(-halfWidht, -halfHeight, -halfDepth)
    );
    // Right upper front.
    result.push_back(
        entityPosition + centerOffset
        + Vector<float, 3>(halfWidht, halfHeight, halfDepth)
    );
    return result;
}

void setMeshBounds(MeshAxisLimitingValues meshAxisLimitingValues) {
    allMeshMaxAbsoluteValues.push_back({});

    allMeshMaxAbsoluteValues[allMeshMaxAbsoluteValues.size() - 1].absolute_x =
        (meshAxisLimitingValues.highest_x - meshAxisLimitingValues.lowest_x)
        / 2.0f;
    allMeshMaxAbsoluteValues[allMeshMaxAbsoluteValues.size() - 1].absolute_y =
        (meshAxisLimitingValues.highest_y - meshAxisLimitingValues.lowest_y)
        / 2.0f;
    allMeshMaxAbsoluteValues[allMeshMaxAbsoluteValues.size() - 1].absolute_z =
        (meshAxisLimitingValues.highest_z - meshAxisLimitingValues.lowest_z)
        / 2.0f;

    allMeshMaxAbsoluteValues[allMeshMaxAbsoluteValues.size() - 1]
        .origin_offset_x =
        (meshAxisLimitingValues.highest_x + meshAxisLimitingValues.lowest_x)
        / 2.0f;
    allMeshMaxAbsoluteValues[allMeshMaxAbsoluteValues.size() - 1]
        .origin_offset_y =
        (meshAxisLimitingValues.highest_y + meshAxisLimitingValues.lowest_y)
        / 2.0f;
    allMeshMaxAbsoluteValues[allMeshMaxAbsoluteValues.size() - 1]
        .origin_offset_z =
        (meshAxisLimitingValues.highest_z + meshAxisLimitingValues.lowest_z)
        / 2.0f;
}

void CreateProjectile(
    const Vector<float, 3>& projectilePosition,
    const Vector<float, 3>& projectileForward,
    const MeshHandle& meshHandle,
    const material& material,
    const damage& damage,
    const EntityLocation& projectileLocation
) {
    ProjectileArchetype* projectileArch =
        static_cast<ProjectileArchetype*>(projectileLocation.arch);
    const uint32_t projectileIndex = projectileLocation.index;

    mesh* projectileMesh = &projectileArch->meshes[projectileIndex];
    projectileMesh->handle = meshHandle;

    ProjectileBundle* projectileBundle =
        &projectileArch->projectileBundles[projectileIndex];
    projectileBundle->material = material;

    transform* rTransformProjectile =
        &projectileArch->transforms[projectileIndex];
    health* projectileHealth = &projectileArch->heath[projectileIndex];
    projectileHealth->maxHealth = 100;
    projectileHealth->currentHealth = 100;

    projectileArch->colliders[projectileIndex].colliders.clear();

    rTransformProjectile->scale = 0.1f;
    rTransformProjectile->position = projectilePosition;
    rTransformProjectile->forward = projectileForward;
    rTransformProjectile->position += rTransformProjectile->forward;

    projectileBundle->damage = damage;
}
}; // namespace glvm

namespace glvm {
ComponentManager* ComponentManager::pInstance_ = nullptr;
std::mutex ComponentManager::Mutex_;

ComponentManager::ComponentManager() = default;

ComponentManager::~ComponentManager() {
    for (int j = 0, iSize_Ordered = worldSparseEntitiesMapToComponents.size();
         j < iSize_Ordered;
         ++j) {
        delete worldSparseEntitiesMapToComponents[j];
        worldSparseEntitiesMapToComponents[j] = nullptr;
    }
    for (int j = 0, iSize_Ordered = worldDenseComponentsMapToEntities.size();
         j < iSize_Ordered;
         ++j) {
        delete worldDenseComponentsMapToEntities[j];
        worldDenseComponentsMapToEntities[j] = nullptr;
    }
}

bool ComponentManager::checkAvailability(
    std::vector<unsigned int>& sparse,
    std::vector<unsigned int>& dense,
    unsigned int entity
) {
    return entity < sparse.size() && sparse[entity] < dense.size()
        && dense[sparse[entity]] == entity;
}

unsigned int ComponentManager::GetContainerID() {
    return componentsContainerID;
}

ComponentManager* ComponentManager::get_instance() {
    std::lock_guard<std::mutex> lock(Mutex_);
    if (pInstance_ == nullptr) {
        pInstance_ = new ComponentManager();
    }
    return pInstance_;
}
} // namespace glvm

glvm::CStack Input_Stack_ {};

int x_pointer;
int y_pointer;

#ifdef __linux__
#endif

glvm::CEvent g_eEvent;
// Contains all maximum absolute axis values.
std::vector<glvm::MeshAxisMaxAbsoluteValues> allMeshMaxAbsoluteValues;

namespace glvm {
Engine* Engine::pInstance_ = nullptr;
std::mutex Engine::Mutex_;

void PlaybackSound(ISoundEngine* _sound_Engine, std::atomic<bool>& runningSound) {
    runningSound = true;
    while (runningSound) {
        _sound_Engine->SoundStream();
    }
}

Engine::Engine() {
    chrono = CTimerCreator().Create();
    soundEngine = CSoundEngineFactory().CreateSoundEngine();

    spatialGridSystem = new SpatialGridSystem();
    collisionSystem = new CCollisionSystem(Input_Stack_);
    movementSystem = new CMovementSystem(Input_Stack_);
    physicsSystem = new CPhysicsSystem(gravity, Input_Stack_);
    projectileSystem = new CProjectileSystem(Input_Stack_);
    damageSystem = new DamageSystem();
    enemySytem = new EnemySystem();
    itemSystem = new ItemSystem();
    procuduralLevelGeneratingSystem = new ProceduralLevelGeneratingSystem();
    inventorySystem = new InventorySystem();

    deltaFrameTime = 0.0;
    g_eEvent.SetEvent(eDEFAULT);

    CSystemManager* pSystem_Manager = CSystemManager::get_instance();

    // Call of ActivateSystem function must be in this order.
    pSystem_Manager->ActivateSystem(procuduralLevelGeneratingSystem);
    pSystem_Manager->ActivateSystem(movementSystem);
    pSystem_Manager->ActivateSystem(enemySytem);
    pSystem_Manager->ActivateSystem(projectileSystem);
    pSystem_Manager->ActivateSystem(spatialGridSystem);
    pSystem_Manager->ActivateSystem(collisionSystem);
    pSystem_Manager->ActivateSystem(damageSystem);
    pSystem_Manager->ActivateSystem(physicsSystem);
    pSystem_Manager->ActivateSystem(inventorySystem);
    pSystem_Manager->ActivateSystem(itemSystem);

    sound_thread = std::thread(
        PlaybackSound,
        std::ref(soundEngine),
        std::ref(runningSound)
    );
    soundEngine->OpenDevice("default");
}

Engine::~Engine() {}

Engine* Engine::get_instance() {
    std::lock_guard<std::mutex> lock(Mutex_);
    if (pInstance_ == nullptr) {
        pInstance_ = new Engine();
    }
    return pInstance_;
}

void Engine::GameLoop() {
    RenderVulkan();
}

void Engine::EventQueueFlush() {}

void Engine::RenderVulkan() {
    CSystemManager* pSystem_Manager = CSystemManager::get_instance();
    bool bGame_Loop_Active = true;

    projectileSystem->textureHandlers = textureHandlers;
    projectileSystem->meshHandlers = meshHandlers;

    enemySytem->textureHandlers = textureHandlers;
    enemySytem->meshHandlers = meshHandlers;

    procuduralLevelGeneratingSystem->meshHandlers = meshHandlers;
    procuduralLevelGeneratingSystem->textureHandlers = textureHandlers;

    inventorySystem->isItemDraged = &draggedItemEntity;
    itemSystem->draggedItemEntity = &draggedItemEntity;

    vulkanRenderer = new CVulkanRenderer();
    vulkanRenderer->initializeTextureData_ = textureVector;
    vulkanRenderer->pathsArray_ = pathsArray_;
    vulkanRenderer->pathsGLTF_ = pathsGLTF_;
    glvm::MeshManager* meshManager = glvm::MeshManager::get_instance();
    vulkanRenderer->SetMeshData(
        meshManager->pathsArray_,
        meshManager->pathsGLTF_
    );

    directionalLightArchetypesNumber = 0;
    world.searchCacheArchetypes(
        directionalLightRequiredMask,
        cachedDirectionalLigthArchetypes,
        directionalLightArchetypesNumber
    );
    vulkanRenderer->directionalLightNumber = directionalLightArchetypesNumber;

    spotLightArchetypesNumber = 0;
    world.searchCacheArchetypes(
        spotLightRequiredMask,
        cachedSpotLigthArchetypes,
        spotLightArchetypesNumber
    );
    vulkanRenderer->spotLightNumber = spotLightArchetypesNumber;

    pointLightArchetypesNumber = 0;
    world.searchCacheArchetypes(
        pointLightRequiredMask,
        cachedPointLigthArchetypes,
        pointLightArchetypesNumber
    );
    vulkanRenderer->pointLightNumber = pointLightArchetypesNumber;

    animationActorsArchetypesNumber = 0;
    world.searchCacheArchetypes(
        animatedActorsRequiredMask,
        cachedAnimationActorsArchetypes,
        animationActorsArchetypesNumber
    );

    staticActorsArchetypesNumber = 0;
    world.searchCacheArchetypes(
        staticActorsRequiredMask,
        cachedStaticActorsArchetypes,
        staticActorsArchetypesNumber
    );

    loadWavefrontObj();
    initializeGLTF();
    initializeFontData();
    vulkanRenderer->run();
    vulkanRenderer->Window->Input_Stack_ = &Input_Stack_;

#ifdef _WIN32
    MSG msg;

    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
#endif

    while (bGame_Loop_Active) {
        deltaFrameTime = chrono->GetElapsed();
        chrono->Reset();
        gravity += deltaFrameTime;

        vulkanRenderer->Window->ClearDisplay();

        vulkanRenderer->Window->HandleEvent(g_eEvent);
        if ((Input_Stack_.SearchElement(EEvents::eGAME_LOOP_KILL))
            == EEvents::eGAME_LOOP_KILL) {
            bGame_Loop_Active = false;
        }

        if ((Input_Stack_.SearchElement(EEvents::eCURSOR_RELEASED))
            == EEvents::eCURSOR_RELEASED) {
            vulkanRenderer->isCursorReleased =
                !vulkanRenderer->isCursorReleased;
            Input_Stack_.Remove(EEvents::eCURSOR_RELEASED);
            Input_Stack_.Remove(EEvents::eCURSOR_RELEASED);
            // The click-to-relock must not trigger on the button that was
            // already held while Esc was pressed.
            Input_Stack_.Remove(EEvents::eMOUSE_LEFT_BUTTON);
        }
        if (vulkanRenderer->isCursorReleased
            && (Input_Stack_.SearchElement(EEvents::eMOUSE_LEFT_BUTTON))
                == EEvents::eMOUSE_LEFT_BUTTON
            && !vulkanRenderer->imguiOverlay->wantsMouse()) {
            vulkanRenderer->isCursorReleased = false;
        }

        if ((Input_Stack_.SearchElement(EEvents::eMOUSE_LEFT_BUTTON))
            == EEvents::eMOUSE_LEFT_BUTTON) {
            isLeftMouseButtonPressed = true;
        } else {
            isLeftMouseButtonPressed = false;
        }

        bool inventoryKeyPressed =
            (Input_Stack_.SearchElement(EEvents::eINVENTORY)
             == EEvents::eINVENTORY);
        if (inventoryKeyPressed && !isInventoryKeyHeld) {
            vulkanRenderer->isInventoryOpened =
                !vulkanRenderer->isInventoryOpened;
            if (vulkanRenderer->isInventoryOpened) {
                pSystem_Manager->DeactivateSystem(
                    DeactivatedSystems::DEACTIVATED_MOVEMENT_SYSTEM
                );
                hud_screen_x = 0.0f;
                hud_screen_y = 0.0f;
            } else {
                pSystem_Manager->ReturnSystemToActivatedState(
                    DeactivatedSystems::DEACTIVATED_MOVEMENT_SYSTEM
                );
            }
        }
        isInventoryKeyHeld = inventoryKeyPressed;
        g_eEvent.SetLastEvent(Input_Stack_);

#ifndef VK_USE_PLATFORM_WAYLAND_KHR
        const bool cursorShouldBeHidden = !vulkanRenderer->isInventoryOpened
            && vulkanRenderer->Window->isFocused
            && !vulkanRenderer->isCursorReleased
            && !vulkanRenderer->imguiOverlay->wantsMouse();
        if (cursorShouldBeHidden && !isCursorHidden) {
            ShowCursor(FALSE);
            isCursorHidden = true;
        } else if (!cursorShouldBeHidden && isCursorHidden) {
            ShowCursor(TRUE);
            isCursorHidden = false;
        }
        if (cursorShouldBeHidden) {
            vulkanRenderer->Window->CursorLock(
                g_eEvent.mousePointerPosition.position_X,
                g_eEvent.mousePointerPosition.position_Y,
                &g_eEvent.mousePointerPosition.offset_X,
                &g_eEvent.mousePointerPosition.offset_Y
            );
        }

        if (wasInventoryOpened && !vulkanRenderer->isInventoryOpened) {
            // Cursor was free while the inventory was open; reset the mouse
            // state so the first locked sample doesn't feed a fake delta to the
            // camera. WindowWinVulkan::CursorLock also discards the >250px
            // teleport on its own.
            g_eEvent.mousePointerPosition.offset_X = 0;
            g_eEvent.mousePointerPosition.offset_Y = 0;
            vulkanRenderer->prev_X = 0.0f;
            vulkanRenderer->prev_Y = 0.0f;
            vulkanRenderer->current_X = 0.0f;
            vulkanRenderer->current_Y = 0.0f;
            movementSystem->prev_X = 0.0f;
            previousMouseOffsetX = 0.0f;
            previousMouseOffsetY = 0.0f;
        }
        wasInventoryOpened = vulkanRenderer->isInventoryOpened;
#else
        if (vulkanRenderer->Window->isFocused) {
            vulkanRenderer->Window->CursorLock(
                g_eEvent.mousePointerPosition.position_X,
                g_eEvent.mousePointerPosition.position_Y,
                &g_eEvent.mousePointerPosition.offset_X,
                &g_eEvent.mousePointerPosition.offset_Y
            );
        }
#endif

        computeHudScreeenCoordinates();
        damageSystem->deltaTime = deltaFrameTime;
        movementSystem->deltaFrameTime = deltaFrameTime;
        movementSystem->gravity = gravity;
        collisionSystem->fDelta_Time_ = deltaFrameTime;
        collisionSystem->gravity = gravity;
        collisionSystem->isInventoryOpened = vulkanRenderer->isInventoryOpened;
        collisionSystem->isLeftMouseButtonPressed = isLeftMouseButtonPressed;
        collisionSystem->isLeftMouseButtonReleased =
            &g_eEvent.isLeftMouseButtonReleased;
        enemySytem->deltaFrameTime = deltaFrameTime;
        enemySytem->soundEngine = soundEngine;
        projectileSystem->deltaFrameTime = deltaFrameTime;
        projectileSystem->soundEngine = soundEngine;
        projectileSystem->isInventoryOpened = vulkanRenderer->isInventoryOpened;
        physicsSystem->fDelta_Time_ = deltaFrameTime;
        physicsSystem->fAcceleration_of_Gravity_ += (deltaFrameTime / 20);
        physicsSystem->gravity = gravity;
        inventorySystem->isInventoryOpened = vulkanRenderer->isInventoryOpened;
        inventorySystem->aspectRate = vulkanRenderer->aspectRate;
        inventorySystem->isLeftMouseButtonReleased =
            &g_eEvent.isLeftMouseButtonReleased;
        inventorySystem->isLeftMouseButtonPressed = isLeftMouseButtonPressed;
        inventorySystem->mouseOffsetX = hud_screen_x;
        inventorySystem->mouseOffsetY = hud_screen_y;
        itemSystem->inputStack = &Input_Stack_;
        itemSystem->isInventoryOpened = vulkanRenderer->isInventoryOpened;
        itemSystem->isLeftMouseButtonReleased =
            &g_eEvent.isLeftMouseButtonReleased;
        itemSystem->isLeftMouseButtonPressed = isLeftMouseButtonPressed;
        itemSystem->mouseOffsetX = hud_screen_x;
        itemSystem->mouseOffsetY = hud_screen_y;
        EnlargeFrameAccumulator(deltaFrameTime);
        pSystem_Manager->Update();
        vulkanRenderer->levelGeneratedVertices =
            procuduralLevelGeneratingSystem->levelGeneratedVertices;
        vulkanRenderer->levelGeneratedIndices =
            procuduralLevelGeneratingSystem->levelGeneratedIndices;
        procuduralLevelGeneratingSystem->levelGeneratedVertices.clear();
        procuduralLevelGeneratingSystem->levelGeneratedIndices.clear();
        vulkanRenderer->draggedItemEntity = draggedItemEntity;
        vulkanRenderer->hud_screen_x = hud_screen_x;
        vulkanRenderer->hud_screen_y = hud_screen_y;
        vulkanRenderer->initializeGameLevelVertices();
        setFrameData();
        if (!vulkanRenderer->isInventoryOpened) {
            SetViewMatrix();
            SetProjectionMatrix();
        }
        vulkanRenderer->draw();
        vulkanRenderer->Window->SwapBuffers();
    }
    delete vulkanRenderer;
}

void Engine::EnlargeFrameAccumulator(float value) {
    animationArchetypesNumber = 0;
    for (uint32_t m = 0; m < world.archetypes.size(); ++m) {
        Archetype* arch = world.archetypes[m];
        uint64_t requiredMask = (1ul << ComponentsIndices::MeshComponent)
            | (1ul << ComponentsIndices::AnimationComponent);

        if (matches_required_mask(arch->mask, requiredMask)) {
            cachedAnimationArchetypes[animationArchetypesNumber] = arch;
            ++animationArchetypesNumber;
        }
    }

    for (uint32_t n = 0; n < animationArchetypesNumber; ++n) {
        Archetype* arch = cachedAnimationArchetypes[n];
        animation* animationView = nullptr;
        mesh* meshView = nullptr;
        if (arch != nullptr) {
            switch (arch->mask) {
                case enemyComponentMask:
                    animationView =
                        static_cast<EnemyArchetype*>(arch)->animations;
                    meshView = static_cast<EnemyArchetype*>(arch)->meshes;
                    break;
                case playerComponentMask:
                    animationView =
                        static_cast<PlayerArchetype*>(arch)->animations;
                    meshView = static_cast<PlayerArchetype*>(arch)->meshes;
                    break;
            }

            for (unsigned int i = 0;
                 i < cachedAnimationArchetypes[n]->entityCount;
                 ++i) {
                if (&meshView[i] != nullptr && &animationView[i] != nullptr) {
                    unsigned int mesh_id = meshView[i].handle.id;
                    if (vulkanRenderer->jointMatricesPerMesh.size() > 0
                        && vulkanRenderer->jointMatricesPerMesh[mesh_id].size()
                            > 0) {
                        animationView[i].frameAccumulator += value;
                    }
                }
            }
        }
    }
}

void Engine::SetViewMatrix() {
    playerArchetypesNumber = 0;
    world.searchCacheArchetypes(
        playerRequiredMask,
        cachedPlayerArchetypes,
        playerArchetypesNumber
    );

    for (uint32_t n = 0; n < playerArchetypesNumber; ++n) {
        Archetype* arch = cachedPlayerArchetypes[n];
        beholder* views =
            (beholder*)arch->components[ComponentsIndices::ViewComponent];
        transform* transfroms =
            (transform*)arch->components[ComponentsIndices::TransformComponent];

        for (uint32_t x = 0; x < arch->entityCount; ++x) {
            beholder* cameraComponent = &views[x];
            transform* _Player = &transfroms[x];

            Matrix<float, 4> viewMatrix_(1.0f);
            const float kSensitivity = 0.1f;
            fYaw = g_eEvent.mousePointerPosition.offset_X;
            fPitch = g_eEvent.mousePointerPosition.offset_Y;
            fYaw *= kSensitivity;
            fPitch *= kSensitivity;

            g_eEvent.mousePointerPosition.pitch = fPitch;
            g_eEvent.mousePointerPosition.yaw = fYaw;

            vulkanRenderer->current_X =
                (float)g_eEvent.mousePointerPosition.offset_X;
            vulkanRenderer->current_Y =
                (float)g_eEvent.mousePointerPosition.offset_Y;
            float delta_x = 0.0f;
            float delta_y = 0.0f;
            if (!vulkanRenderer->isInventoryOpened) {
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
                delta_x = vulkanRenderer->current_X;
                delta_y = vulkanRenderer->current_Y;
#else
                delta_x = vulkanRenderer->current_X - vulkanRenderer->prev_X;
                delta_y = vulkanRenderer->current_Y - vulkanRenderer->prev_Y;
                delta_y *= -1.0f;
#endif
            }

            const Vector<float, 3> rightVec = Cross(
                cameraComponent->forward,
                Vector<float, 3>(0.0f, -1.0f, 0.0)
            );
            const Vector<float, 3> newUpVec =
                Cross(rightVec, cameraComponent->forward);
            // 1. The mouse direction determines the "intended direction of
            // rotation" for the object.
            // 2. The camera is "looking forward."
            // 3. To make the object "rotate as if the mouse is pushing it," you
            // need to rotate it around an axis that is perpendicular to both
            // the view direction and the mouse movement.
            const Vector<float, 3> rotateAxis = Normalize(Cross(
                cameraComponent->forward,
                rightVec * delta_x + newUpVec * delta_y
            ));

            if (VecLength(rotateAxis) >= 0.001f) {
                // A vector in the screen's tangent plane: it indicates the
                // direction in which the mouse moved, but expressed in world
                // (or 3D) space.
                float rotationAngle =
                    sqrt(delta_y * delta_y + delta_x * delta_x);
                constexpr float angleScale = 0.05f;
                rotationAngle = Radians(rotationAngle * angleScale);
                // Quaternions need devision by 2.
                constexpr float quatAngleCorrection = 0.5f;
                [[maybe_unused]] const float sinRotationAngle =
                    sinf(rotationAngle * quatAngleCorrection);
                point appliedRotationPoint =
                    exp(rotationAngle,
                        rline {
                            .rx = -rotateAxis.m_vector[0],
                            .ry = -rotateAxis.m_vector[1],
                            .rz = -rotateAxis.m_vector[2]
                        })
                    >> point {
                        .x = cameraComponent->forward[0],
                        .y = cameraComponent->forward[1],
                        .z = cameraComponent->forward[2],
                        .w = 1.0f
                    };
                vulkanRenderer->forward[0] = appliedRotationPoint.x;
                vulkanRenderer->forward[1] = appliedRotationPoint.y;
                vulkanRenderer->forward[2] = appliedRotationPoint.z;
            }
            cameraComponent->forward = Normalize(vulkanRenderer->forward);
            // Pitch limit by ANGLE, not pixels: independent of screen
            // resolution and mouse sensitivity. Keeps the camera off the
            // vertical pole, where the view basis Cross(forward, up)
            // degenerates and the world starts rolling.
            constexpr float maxPitchSin = 0.9999996f; // sin(89.95°).
            if (cameraComponent->forward[1] > maxPitchSin) {
                cameraComponent->forward[1] = maxPitchSin;
            } else if (cameraComponent->forward[1] < -maxPitchSin) {
                cameraComponent->forward[1] = -maxPitchSin;
            }
            cameraComponent->forward = Normalize(cameraComponent->forward);
            _Player->forward = cameraComponent->forward;
            Matrix<float, 4> view = LookAtMain(
                cameraComponent->Position + _Player->position,
                cameraComponent->Position + _Player->position
                    + cameraComponent->forward,
                Vector<float, 3>(0.0f, -1.0f, 0.0)
            );
            for (unsigned int i = 0; i < 4; ++i) {
                for (unsigned int j = 0; j < 4; ++j) {
                    viewMatrix_[i][j] = view[i][j];
                }
            }

            vulkanRenderer->viewMatrix = viewMatrix_;

            vulkanRenderer->prev_Y =
                (float)g_eEvent.mousePointerPosition.offset_Y;
            vulkanRenderer->prev_X =
                (float)g_eEvent.mousePointerPosition.offset_X;
        }
    }
}

void Engine::SetProjectionMatrix() {
    Matrix<float, 4> tProjection_Matrix =
        Perspective(Radians(90.0f), vulkanRenderer->aspectRate, 0.1f, 100.0f);
    vulkanRenderer->projectionMatrix = tProjection_Matrix;
    vulkanRenderer->projectionMatrix[1][1] *= 1.0f;
}

[[nodiscard]] std::vector<Matrix<float, 4>> Engine::updateAnimationFrames(
    [[maybe_unused]] animation* animationComponent,
    [[maybe_unused]] unsigned int meshID
) {
    if (vulkanRenderer->jointMatricesPerMesh.size() > 0
        && vulkanRenderer->jointMatricesPerMesh[meshID].size() > 0
        && animationComponent->frameAccumulator
            >= vulkanRenderer
                    ->frames[meshID][animationComponent->currentAnimationFrame]
                * 1.0f) {
        ++animationComponent->currentAnimationFrame;
        if (vulkanRenderer->jointMatricesPerMesh[meshID].size() > 0
            && animationComponent->currentAnimationFrame
                == vulkanRenderer->frames[meshID].size()) {
            animationComponent->currentAnimationFrame = 0;
            animationComponent->frameAccumulator = 0.0f;
        }
    }

    unsigned int joinMatricesDataSize {};
    if (vulkanRenderer->jointMatricesPerMesh.size() > 0) {
        joinMatricesDataSize =
            vulkanRenderer->jointMatricesPerMesh[meshID].size();
    }

    std::vector<Matrix<float, 4>> jointMatrices;
    if (joinMatricesDataSize == 0) {
        jointMatrices.resize(MAX_JOINTS_NUMBER);
        for (unsigned int i = 0; i < MAX_JOINTS_NUMBER; ++i) {
            Matrix<float, 4> unitMatrix(1.0f);
            jointMatrices[i] = unitMatrix;
        }

    } else {
        jointMatrices.resize(MAX_JOINTS_NUMBER);
        for (unsigned int i = 0; i < joinMatricesDataSize; ++i) {
            if (meshID >= vulkanRenderer->jointMatricesPerMesh.size()) {
                std::cout << "OUTER ARRAY OVERFLOW" << std::endl;
                throw("sdfsdf");
            } else if (i >= vulkanRenderer->jointMatricesPerMesh[meshID].size()) {
                std::cout << "MIDDLE ARRAY OVERFLOW" << std::endl;
                throw("sdfsdf");
            } else if (
                animationComponent->currentAnimationFrame
                >= vulkanRenderer->jointMatricesPerMesh[meshID][i].size()
            ) {
                std::cout << "frame: "
                          << animationComponent->currentAnimationFrame
                          << std::endl;
                std::cout
                    << "array size: "
                    << vulkanRenderer->jointMatricesPerMesh[meshID][i].size()
                    << std::endl;
                std::cout << "frames number: "
                          << vulkanRenderer->frames[meshID].size() << std::endl;
                std::cout << "INNER ARRAY OVERFLOW" << std::endl;
                throw("sdfsdf");
            }

            jointMatrices[i] =
                vulkanRenderer->jointMatricesPerMesh
                    [meshID][i][animationComponent->currentAnimationFrame];
        }

        for (uint32_t j = joinMatricesDataSize; j < MAX_JOINTS_NUMBER; ++j) {
            Matrix<float, 4> unitMatrix(1.0f);
            jointMatrices[j] = unitMatrix;
        }
    }

    return jointMatrices;
}

Matrix<float, 4> Engine::updateDirectionalLightSpaceMatrixShadowMapUBO(
    directionalLight* directionalLightComponent
) {
    float nearPlaneFlatShadowMap = 5.5f;
    float farPlaneFlatShadowMap = 100.0f;
    Matrix<float, 4> directionalProjectionMatrixLight = ortho(
        -50.0f,
        50.0f,
        -50.0f,
        50.0f,
        nearPlaneFlatShadowMap,
        farPlaneFlatShadowMap
    );

    Vector<float, 3> positionVectorLight = directionalLightComponent->position;
    Vector<float, 3> directionVectorLight =
        directionalLightComponent->direction;

    Matrix<float, 4> viewMatrixLight = LookAtMain(
        positionVectorLight,
        directionVectorLight,
        {0.0f, -1.0f, 0.0f}
    );
    return viewMatrixLight * directionalProjectionMatrixLight;
}

Matrix<float, 4> Engine::updateSpotLightSpaceMatrixShadowMapUBO(
    spotLight* spotLightComponent
) {
    float nearPlaneFlatShadowMap = 0.5f;
    float farPlaneFlatShadowMap = 100.0f;
    Matrix<float, 4> spotProjectionMatrixLight = Perspective(
        Radians(90.0f),
        (float)SHADOW_MAP_SIZE / (float)SHADOW_MAP_SIZE,
        nearPlaneFlatShadowMap,
        farPlaneFlatShadowMap
    );

    Vector<float, 3> positionVectorLight = spotLightComponent->position;
    Vector<float, 3> directionVectorLight = spotLightComponent->direction;
    Matrix<float, 4> viewMatrixLight = LookAtMain(
        positionVectorLight,
        directionVectorLight,
        {0.0f, -1.0f, 0.0f}
    );
    return viewMatrixLight * spotProjectionMatrixLight;
}

Matrix<float, 4> Engine::updatePointLightSpaceMatrixShadowMapUBO(
    pointLight* pointLightComponent,
    uint32_t layer
) {
    Vector<float, 3> positionVectorLight = pointLightComponent->position;
    Vector<float, 3> directionalVectorLight =
        Vector<float, 3>(0.0f, 0.0f, 0.0f);
    Vector<float, 3> upVector = {0.0, 0.0, 0.0};

    switch (layer) {
        case 0:
            // Positive X.
            directionalVectorLight =
                positionVectorLight + Vector<float, 3>(1.0f, 0.0f, 0.0f);
            upVector = Vector<float, 3>(0.0f, -1.0f, 0.0f);
            break;
        case 1:
            // Negative X.
            directionalVectorLight =
                positionVectorLight + Vector<float, 3>(-1.0f, 0.0f, 0.0f);
            upVector = Vector<float, 3>(0.0f, -1.0f, 0.0f);
            break;
        case 2:
            // Positive Y.
            directionalVectorLight =
                positionVectorLight + Vector<float, 3>(0.0f, 1.0f, 0.0f);
            upVector = Vector<float, 3>(0.0f, 0.0f, 1.0f);
            break;
        case 3:
            // Negative Y.
            directionalVectorLight =
                positionVectorLight + Vector<float, 3>(0.0f, -1.0f, 0.0f);
            upVector = Vector<float, 3>(0.0f, 0.0f, -1.0f);
            break;
        case 4:
            // Positive Z.
            directionalVectorLight =
                positionVectorLight + Vector<float, 3>(0.0f, 0.0f, 1.0f);
            upVector = Vector<float, 3>(0.0f, -1.0f, 0.0f);
            break;
            // Negative Z.
        case 5:
            directionalVectorLight =
                positionVectorLight + Vector<float, 3>(0.0f, 0.0f, -1.0f);
            upVector = Vector<float, 3>(0.0f, -1.0f, 0.0f);
            break;
        default:
            break;
    }

    Matrix<float, 4> projectionMatrixCubeShadowMap = Perspective(
        Radians(90.0f),
        (float)SHADOW_MAP_SIZE / (float)SHADOW_MAP_SIZE,
        0.3f,
        100.0f
    );

    Matrix<float, 4> viewMatrixLight =
        LookAtMain(positionVectorLight, directionalVectorLight, upVector);

    return viewMatrixLight * projectionMatrixCubeShadowMap;
}

[[nodiscard]] SlotData Engine::updateDataUBO_UI(
    const unsigned int currentInventoryRow,
    const unsigned int currentInventoryColumn,
    inventory* inventoryComponent,
    transform* slotTransfromComponent,
    mesh* meshComponent
) {
    SlotData hudUBO {};
    Matrix<float, 4> model(1.0);
    const float fullSlotScale = meshComponent->gltf
        ? inventoryComponent->slotScale * 2.0f
        : inventoryComponent->slotScale;
    const float x = slotTransfromComponent->position[0]
        + currentInventoryColumn * fullSlotScale;
    const float y_scaleMultilayer = vulkanRenderer->aspectRate * fullSlotScale;
    const float y = slotTransfromComponent->position[1]
        + currentInventoryRow * y_scaleMultilayer;
    const float inventorySlotScale = inventoryComponent->slotScale;
    model[0][0] = inventorySlotScale;
    model[1][1] = inventorySlotScale;
    model[2][2] = inventorySlotScale;
    model[3][0] = x;
    model[3][1] = y;
    model[3][2] = 0.1f;

    hudUBO.model = model;

    bool highLightedSlot = false;
    for (unsigned int i = 0; i < inventoryComponent->highlightedSlots.size();
         ++i) {
        if (inventoryComponent->highlightedSlots[i]
            == currentInventoryRow * inventoryComponent->col
                + currentInventoryColumn) {
            highLightedSlot = true;
            break;
        } else {
            continue;
        }
    }

    if (inventoryComponent->highlightedSlots.size() > 0) {
        if (highLightedSlot) {
            if (inventoryComponent->isAvailableHighlightedSlots) {
                hudUBO.color = {0.0, 0.3, 0.0};
            } else {
                hudUBO.color = {0.3, 0.0, 0.0};
            }
        }
    } else {
        hudUBO.color = {0.0, 0.0, 0.0};
    }

    return hudUBO;
}

Matrix<float, 4> Engine::updateDataUBO_IconsUI(
    transform* itemTransfromComponent,
    [[maybe_unused]] collider* itemColliderComponent,
    item* itemComponent,
    const unsigned int rowInventory,
    const unsigned int columnInventory,
    transform* inventoryTransformComponent,
    mesh* itemMesh,
    int itemEntity
) {
    float x_result_offset = 0.0f;
    float y_result_offset = 0.0f;
    if (itemComponent->occupiedSlots.size() == 0) {
    } else {
        const unsigned int inventorySlotEntity_0 =
            itemComponent->occupiedSlots[0];
        const unsigned int inventorySlotEntity_3 =
            itemComponent->occupiedSlots.back();
        const unsigned int rowIndexFirstSlot =
            inventorySlotEntity_0 / rowInventory;
        const unsigned int colIndexFirstSlot =
            inventorySlotEntity_0 % columnInventory;
        const unsigned int rowIndexSecondSlot =
            inventorySlotEntity_3 / rowInventory;
        const unsigned int colIndexSecondSlot =
            inventorySlotEntity_3 % columnInventory;

        const float itemScale = itemTransfromComponent->scale;
        const float fullSlotScale =
            itemMesh->gltf ? itemScale * 2.0f : itemScale;
        // Eather division by 2.0f using multiply on 0.5f.
        constexpr float centreMultiplayer = 0.5f;
        x_result_offset = inventoryTransformComponent->position[0]
            + (colIndexFirstSlot * fullSlotScale
               + colIndexSecondSlot * fullSlotScale)
                * centreMultiplayer;
        y_result_offset = inventoryTransformComponent->position[1]
            + (rowIndexFirstSlot * fullSlotScale
               + rowIndexSecondSlot * fullSlotScale)
                * centreMultiplayer * vulkanRenderer->aspectRate;
    }
    float itemScale = itemTransfromComponent->scale;

    if (draggedItemEntity != itemEntity) {
        itemTransfromComponent->position =
            Vector<float, 3>(x_result_offset, y_result_offset, 0.1f);
    } else {
        itemScale *= 1.1f;
        itemTransfromComponent->position[2] = 0.0f;
    }
    Matrix<float, 4> model(1.0);
    model[0][0] = itemScale * itemComponent->itemSlotType.width;
    model[1][1] = itemScale * itemComponent->itemSlotType.height;
    model[2][2] = 0.0f;
    model[3][0] = itemTransfromComponent->position[0];
    model[3][1] = itemTransfromComponent->position[1];
    model[3][2] = itemTransfromComponent->position[2];

    return model;
}

Matrix<float, 4> Engine::updateDataHudScreenUBO(transform* cursorTransform) {
    Matrix<float, 4> model;
    Vector<float, 3> defaultPosition = Vector<float, 3>(0.0, 0.0, 0.0);

    float hudScreenX = hud_screen_x;
#ifndef VK_USE_PLATFORM_WAYLAND_KHR
    hudScreenX = -hud_screen_x;
#endif

    cursorTransform->position[0] = hudScreenX;
    cursorTransform->position[1] = -hud_screen_y;

    if (!vulkanRenderer->isInventoryOpened
        && !vulkanRenderer->isCursorReleased) {
        model[3][0] = defaultPosition[0];
        model[3][1] = defaultPosition[1];
        model[3][2] = defaultPosition[2];
        model[0][0] = cursorTransform->scale;
        model[1][1] = cursorTransform->scale;
        model[2][2] = cursorTransform->scale;
        model[3][3] = 1.0f;
    } else {
        defaultPosition[0] = hudScreenX;
        defaultPosition[1] = -hud_screen_y;

        model[3][0] = defaultPosition[0];
        model[3][1] = defaultPosition[1];
        model[3][2] = defaultPosition[2];
        model[0][0] = cursorTransform->scale;
        model[1][1] = cursorTransform->scale;
        model[2][2] = cursorTransform->scale;
        model[3][3] = 1.0f;
    }

    return model;
}

void Engine::setFrameData() {
    vulkanRenderer->directionalLights.clear();
    directionalLightArchetypesNumber = 0;
    world.searchCacheArchetypes(
        directionalLightRequiredMask,
        cachedDirectionalLigthArchetypes,
        directionalLightArchetypesNumber
    );

    uint32_t directionalLightCounter = 0;
    for (uint32_t x = 0; x < directionalLightArchetypesNumber; ++x) {
        Archetype* arch = cachedDirectionalLigthArchetypes[x];
        directionalLight* directionalLights =
            (directionalLight*)
                arch->components[ComponentsIndices::DirectionalLightComponent];

        for (uint32_t x1 = 0; x1 < arch->entityCount; ++x1) {
            if (directionalLights) {
                vulkanRenderer->directionalLights.push_back({});
                directionalLight* directionalLightComponent =
                    &directionalLights[x1];
                vulkanRenderer->directionalLights[directionalLightCounter]
                    .DirectionalLightSpaceMatrix =
                    updateDirectionalLightSpaceMatrixShadowMapUBO(
                        directionalLightComponent
                    );
                vulkanRenderer->directionalLights[directionalLightCounter]
                    .position = Vector<float, 4>(
                    directionalLightComponent->position[0],
                    directionalLightComponent->position[1],
                    directionalLightComponent->position[2],
                    0.0
                );
                vulkanRenderer->directionalLights[directionalLightCounter]
                    .direction = Vector<float, 4>(
                    directionalLightComponent->direction[0],
                    directionalLightComponent->direction[1],
                    directionalLightComponent->direction[2],
                    0.0
                );
                vulkanRenderer->directionalLights[directionalLightCounter]
                    .ambient = Vector<float, 4>(
                    directionalLightComponent->ambient[0],
                    directionalLightComponent->ambient[1],
                    directionalLightComponent->ambient[2],
                    0.0
                );
                vulkanRenderer->directionalLights[directionalLightCounter]
                    .diffuse = Vector<float, 4>(
                    directionalLightComponent->diffuse[0],
                    directionalLightComponent->diffuse[1],
                    directionalLightComponent->diffuse[2],
                    0.0
                );
                vulkanRenderer->directionalLights[directionalLightCounter]
                    .specular = Vector<float, 4>(
                    directionalLightComponent->specular[0],
                    directionalLightComponent->specular[1],
                    directionalLightComponent->specular[2],
                    0.0
                );
                ++directionalLightCounter;
            }
        }
    }

    vulkanRenderer->spotLights.clear();
    spotLightArchetypesNumber = 0;
    world.searchCacheArchetypes(
        spotLightRequiredMask,
        cachedSpotLigthArchetypes,
        spotLightArchetypesNumber
    );

    uint32_t spotLightCounter = 0;
    for (uint32_t x = 0; x < spotLightArchetypesNumber; ++x) {
        Archetype* arch = cachedSpotLigthArchetypes[x];
        spotLight* spotLights =
            (spotLight*)arch->components[ComponentsIndices::SpotLightComponent];

        for (uint32_t x1 = 0; x1 < arch->entityCount; ++x1) {
            if (spotLights) {
                vulkanRenderer->spotLights.push_back({});
                spotLight* spotLightComponent = &spotLights[x1];
                vulkanRenderer->spotLights[spotLightCounter]
                    .SpotLigthSpaceMatrix =
                    updateSpotLightSpaceMatrixShadowMapUBO(spotLightComponent);
                vulkanRenderer->spotLights[spotLightCounter].position =
                    spotLightComponent->position;
                vulkanRenderer->spotLights[spotLightCounter].direction =
                    spotLightComponent->direction;
                vulkanRenderer->spotLights[spotLightCounter].cutOff =
                    spotLightComponent->cutOff;
                vulkanRenderer->spotLights[spotLightCounter].outerCutOff =
                    spotLightComponent->outerCutOff;
                vulkanRenderer->spotLights[spotLightCounter].ambient =
                    spotLightComponent->ambient;
                vulkanRenderer->spotLights[spotLightCounter].diffuse =
                    spotLightComponent->diffuse;
                vulkanRenderer->spotLights[spotLightCounter].specular =
                    spotLightComponent->specular;
                vulkanRenderer->spotLights[spotLightCounter].constant =
                    spotLightComponent->constant;
                vulkanRenderer->spotLights[spotLightCounter].linear =
                    spotLightComponent->linear;
                vulkanRenderer->spotLights[spotLightCounter].quadratic =
                    spotLightComponent->quadratic;
                ++spotLightCounter;
            }
        }
    }

    vulkanRenderer->pointLights.clear();
    pointLightArchetypesNumber = 0;
    world.searchCacheArchetypes(
        pointLightRequiredMask,
        cachedPointLigthArchetypes,
        pointLightArchetypesNumber
    );

    uint32_t pointLightCounter = 0;
    for (uint32_t x = 0; x < pointLightArchetypesNumber; ++x) {
        Archetype* arch = cachedPointLigthArchetypes[x];
        pointLight* pointLights =
            (pointLight*)
                arch->components[ComponentsIndices::PointLightComponent];

        for (uint32_t x1 = 0; x1 < arch->entityCount; ++x1) {
            if (pointLights) {
                vulkanRenderer->pointLights.push_back({});
                pointLight* pointLightComponent = &pointLights[x1];
                uint32_t maxCubeMapLayers = 6;
                // 6 is a number of cube map layers.
                for (uint32_t cubeMapLayerCounter = 0;
                     cubeMapLayerCounter < maxCubeMapLayers;
                     ++cubeMapLayerCounter) {
                    vulkanRenderer->pointLights[pointLightCounter]
                        .pointLightSpaceMatrix[cubeMapLayerCounter] =
                        updatePointLightSpaceMatrixShadowMapUBO(
                            pointLightComponent,
                            cubeMapLayerCounter
                        );
                }
                vulkanRenderer->pointLights[pointLightCounter].position =
                    pointLightComponent->position;
                vulkanRenderer->pointLights[pointLightCounter].ambient =
                    pointLightComponent->ambient;
                vulkanRenderer->pointLights[pointLightCounter].diffuse =
                    pointLightComponent->diffuse;
                vulkanRenderer->pointLights[pointLightCounter].specular =
                    pointLightComponent->specular;
                vulkanRenderer->pointLights[pointLightCounter].constant =
                    pointLightComponent->constant;
                vulkanRenderer->pointLights[pointLightCounter].linear =
                    pointLightComponent->linear;
                vulkanRenderer->pointLights[pointLightCounter].quadratic =
                    pointLightComponent->quadratic;
                ++pointLightCounter;
            }
        }
    }

    vulkanRenderer->healthBars.clear();
    healthBarsArchetypesNumber = 0;
    world.searchCacheArchetypes(
        healthBarsRequiredMask,
        cachedHealthBarsArchetypes,
        healthBarsArchetypesNumber
    );

    uint32_t healthBarCounter = 0;
    for (uint32_t x = 0; x < healthBarsArchetypesNumber; ++x) {
        Archetype* arch = cachedHealthBarsArchetypes[x];
        transform* healthBarTransforms =
            (transform*)arch->components[ComponentsIndices::TransformComponent];
        mesh* healthBarMeshes =
            (mesh*)arch->components[ComponentsIndices::MeshComponent];
        health* healthBars =
            (health*)arch->components[ComponentsIndices::HealthComponent];

        unsigned int uiVertexId = 0;
        if (matches_required_mask(arch->mask, playerComponentMask)) {
            uiVertexId = healthBarMeshes[0].handle.id;
        }

        for (unsigned int i = 0; i < arch->entityCount; ++i) {
            vulkanRenderer->healthBars.push_back({});
            transform* transformComponent = &healthBarTransforms[i];
            health* healthComponent = &healthBars[i];
            vulkanRenderer->healthBars[healthBarCounter].meshID = uiVertexId;
            vulkanRenderer->healthBars[healthBarCounter].position =
                transformComponent->position;
            vulkanRenderer->healthBars[healthBarCounter].maxHealth =
                healthComponent->maxHealth;
            vulkanRenderer->healthBars[healthBarCounter].currentHealth =
                healthComponent->currentHealth;
            ++healthBarCounter;
        }
    }

    vulkanRenderer->fonts.clear();
    fontsArchetypesNumber = 0;
    world.searchCacheArchetypes(
        fontRequiredMask,
        cachedFontsArchetypes,
        fontsArchetypesNumber
    );

    uint32_t fontCounter = 0;
    for (uint32_t x = 0; x < fontsArchetypesNumber; ++x) {
        Archetype* arch = cachedFontsArchetypes[x];
        transform* fontTransforms =
            (transform*)arch->components[ComponentsIndices::TransformComponent];
        font* fonts = (font*)arch->components[ComponentsIndices::FontComponent];

        for (unsigned int i = 0; i < arch->entityCount; ++i) {
            vulkanRenderer->fonts.push_back({});
            font* fontComponent = &fonts[i];
            transform* transformComponent = &fontTransforms[i];
            vulkanRenderer->fonts[fontCounter].position =
                transformComponent->position;
            vulkanRenderer->fonts[fontCounter].font_string =
                fontComponent->font_string;
            vulkanRenderer->fonts[fontCounter].lifeTime =
                fontComponent->lifeTime;
            ++fontCounter;
        }
    }

    if (vulkanRenderer->isInventoryOpened) {
        vulkanRenderer->inventories.clear();
        uint32_t inventoryCounter = 0;
        inventoryArchetypesNumber = 0;
        world.searchCacheArchetypes(
            inventoryRequiredMask,
            cachedInventoryArchetypes,
            inventoryArchetypesNumber
        );

        for (uint32_t x = 0; x < inventoryArchetypesNumber; ++x) {
            Archetype* arch = cachedInventoryArchetypes[x];
            transform* inventoryTransforms =
                (transform*)
                    arch->components[ComponentsIndices::TransformComponent];
            inventory* inventoryData =
                (inventory*)
                    arch->components[ComponentsIndices::InventoryComponent];
            material* inventoryMaterials =
                (material*)
                    arch->components[ComponentsIndices::MaterialComponent];
            mesh* inventoryMeshes =
                (mesh*)arch->components[ComponentsIndices::MeshComponent];

            if (inventoryTransforms && inventoryMaterials && inventoryData
                && inventoryMeshes) {
                for (unsigned int i = 0; i < arch->entityCount; ++i) {
                    vulkanRenderer->inventories.push_back({});
                    inventory* inventoryComponent = &inventoryData[i];
                    unsigned int inventoryTextureID =
                        inventoryMaterials[i].diffuseTextureID_.id;
                    unsigned int meshID = inventoryComponent->slotMeshID.id;
                    vulkanRenderer->inventories[inventoryCounter]
                        .inventoryTextureID = inventoryTextureID;
                    vulkanRenderer->inventories[inventoryCounter].meshID =
                        meshID;
                    vulkanRenderer->inventories[inventoryCounter].row =
                        inventoryComponent->row;
                    vulkanRenderer->inventories[inventoryCounter].col =
                        inventoryComponent->col;
                    vulkanRenderer->inventories[inventoryCounter]
                        .slotData.clear();
                    for (unsigned int j = 0; j < inventoryComponent->row; ++j) {
                        for (unsigned int m = 0; m < inventoryComponent->col;
                             ++m) {
                            transform* slotTransformComponent =
                                &inventoryTransforms[i];
                            vulkanRenderer->inventories[inventoryCounter]
                                .slotData.push_back({});
                            vulkanRenderer->inventories[inventoryCounter]
                                .slotData[j * inventoryComponent->col + m] =
                                updateDataUBO_UI(
                                    j,
                                    m,
                                    inventoryComponent,
                                    slotTransformComponent,
                                    &inventoryMeshes[i]
                                );
                        }
                    }
                    ++inventoryCounter;
                }

                for (unsigned int i = 0; i < arch->entityCount; ++i) {
                    inventory* inventoryComponent = &inventoryData[i];
                    transform* inventoryTransformComponent =
                        &inventoryTransforms[i];

                    vulkanRenderer->items.clear();
                    uint32_t itemCounter = 0;
                    itemArchetypesNumber = 0;
                    world.searchCacheArchetypes(
                        itemRequiredMask,
                        cachedItemArchetypes,
                        itemArchetypesNumber
                    );

                    for (uint32_t c = 0; c < itemArchetypesNumber; ++c) {
                        Archetype* arch = cachedItemArchetypes[c];
                        transform* itemTransforms =
                            (transform*)arch->components
                                [ComponentsIndices::TransformComponent];
                        item* items =
                            (item*)arch
                                ->components[ComponentsIndices::ItemComponent];
                        material* itemMaterials =
                            (material*)arch->components
                                [ComponentsIndices::MaterialComponent];
                        mesh* itemMeshes =
                            (mesh*)arch
                                ->components[ComponentsIndices::MeshComponent];
                        collider* itemColliders =
                            (collider*)arch->components
                                [ComponentsIndices::ColliderComponent];

                        if (itemTransforms && itemMaterials && itemMeshes
                            && itemColliders && items) {
                            for (unsigned int a = 0; a < arch->entityCount;
                                 ++a) {
                                item* itemComponent = &items[a];
                                if (!itemComponent->isActor) {
                                    vulkanRenderer->items.push_back({});
                                    unsigned int meshID =
                                        itemMeshes[a].handle.id;
                                    unsigned int diffuseTextureID =
                                        itemMaterials[a].diffuseTextureID_.id;
                                    vulkanRenderer->items[itemCounter].meshID =
                                        meshID;
                                    vulkanRenderer->items[itemCounter]
                                        .diffuseTextureID = diffuseTextureID;
                                    transform* itemTransformComponent =
                                        &itemTransforms[a];
                                    collider* itemColliderComponent =
                                        &itemColliders[a];

                                    if (itemTransformComponent == nullptr) {
                                        std::cout << "NULL POINTER"
                                                  << std::endl;
                                    }

                                    uint32_t itemEntity = arch->entities[a];
                                    vulkanRenderer->items[itemCounter].model =
                                        updateDataUBO_IconsUI(
                                            itemTransformComponent,
                                            itemColliderComponent,
                                            itemComponent,
                                            inventoryComponent->row,
                                            inventoryComponent->col,
                                            inventoryTransformComponent,
                                            &itemMeshes[a],
                                            itemEntity
                                        );
                                    ++itemCounter;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    vulkanRenderer->crosshairs.clear();
    crosshairActorsArchetypesNumber = 0;
    world.searchCacheArchetypes(
        crosshairRequiredMask,
        cachedCrosshairActorsArchetypes,
        crosshairActorsArchetypesNumber
    );

    for (uint32_t x = 0; x < crosshairActorsArchetypesNumber; ++x) {
        Archetype* arch = cachedCrosshairActorsArchetypes[x];
        transform* crosshairTransforms =
            (transform*)arch->components[ComponentsIndices::TransformComponent];
        mesh* crosshairMeshes =
            (mesh*)arch->components[ComponentsIndices::MeshComponent];

        for (unsigned int i = 0; i < arch->entityCount; ++i) {
            vulkanRenderer->crosshairs.push_back({});
            transform* cursorTransform = &crosshairTransforms[i];
            unsigned int meshID = crosshairMeshes[i].handle.id;
            vulkanRenderer->crosshairs[i].meshID = meshID;
            vulkanRenderer->crosshairs[i].model =
                updateDataHudScreenUBO(cursorTransform);
        }
    }

    vulkanRenderer->actors.clear();
    levelChunkActorsArchetypesNumber = 0;
    world.searchCacheArchetypes(
        levelChunkRequiredMask,
        cachedLevelChunkActorsArchetypes,
        levelChunkActorsArchetypesNumber
    );

    uint32_t levelChunkActorsCounter = 0;
    for (uint32_t x = 0; x < levelChunkActorsArchetypesNumber; ++x) {
        Archetype* arch = cachedLevelChunkActorsArchetypes[x];
        transform* levelChunkTransforms =
            (transform*)arch->components[ComponentsIndices::TransformComponent];
        mesh* levelChunkMeshes =
            (mesh*)arch->components[ComponentsIndices::MeshComponent];
        material* levelChunkMaterials =
            (material*)arch->components[ComponentsIndices::MaterialComponent];
        rotation* levelChunkRotations =
            (rotation*)arch->components[ComponentsIndices::RotationComponent];
        levelChunkTagComponent* levelChunks =
            (levelChunkTagComponent*)
                arch->components[ComponentsIndices::LevelChunkTagComponent];

        std::vector<Matrix<float, 4>> jointMatrices;
        jointMatrices.resize(MAX_JOINTS_NUMBER);
        for (unsigned int i = 0; i < MAX_JOINTS_NUMBER; ++i) {
            Matrix<float, 4> unitMatrix(1.0f);
            jointMatrices[i] = unitMatrix;
        }

        for (uint32_t n = 0; n < arch->entityCount; ++n) {
            vulkanRenderer->actors.push_back({});
            transform* transformComponent = &levelChunkTransforms[n];
            material* materialComponent = &levelChunkMaterials[n];
            rotation* rotationComponent = &levelChunkRotations[n];
            if (levelChunkTransforms && levelChunkMaterials && levelChunks
                && levelChunkRotations && levelChunkMeshes) {
                unsigned int meshID = levelChunkMeshes[n].handle.id;
                vulkanRenderer->actors[levelChunkActorsCounter].modelMatrix =
                    computeModelMatrix(transformComponent, rotationComponent);
                vulkanRenderer->actors[levelChunkActorsCounter].jointMatrices =
                    jointMatrices;
                vulkanRenderer->actors[levelChunkActorsCounter].meshID = meshID;
                vulkanRenderer->actors[levelChunkActorsCounter]
                    .diffuseTextureIndex =
                    materialComponent->diffuseTextureID_.id;
                vulkanRenderer->actors[levelChunkActorsCounter]
                    .specularTextureIndex =
                    materialComponent->specularTextureID_.id;
                vulkanRenderer->actors[levelChunkActorsCounter].ambient =
                    materialComponent->ambient;
                vulkanRenderer->actors[levelChunkActorsCounter].shininess =
                    materialComponent->shininess;
                ++levelChunkActorsCounter;
            }
        }
    }

    animationActorsArchetypesNumber = 0;
    world.searchCacheArchetypes(
        animationRequiredMask,
        cachedAnimationArchetypes,
        animationActorsArchetypesNumber
    );

    uint32_t animationActorsCounter = levelChunkActorsCounter;
    for (uint32_t x = 0; x < animationActorsArchetypesNumber; ++x) {
        Archetype* arch = cachedAnimationActorsArchetypes[x];
        transform* actorTransforms =
            (transform*)arch->components[ComponentsIndices::TransformComponent];
        mesh* actorMeshes =
            (mesh*)arch->components[ComponentsIndices::MeshComponent];
        material* actorMaterials =
            (material*)arch->components[ComponentsIndices::MaterialComponent];
        rotation* actorRotations =
            (rotation*)arch->components[ComponentsIndices::RotationComponent];
        animation* actorAnimations =
            (animation*)arch->components[ComponentsIndices::AnimationComponent];

        for (uint32_t n = 0; n < arch->entityCount; ++n) {
            vulkanRenderer->actors.push_back({});
            transform* transformComponent = &actorTransforms[n];
            material* materialComponent = &actorMaterials[n];
            [[maybe_unused]] animation* animationComponent =
                &actorAnimations[n];
            rotation* rotationComponent = &actorRotations[n];
            if (actorTransforms && actorMaterials && actorAnimations
                && actorRotations) {
                unsigned int meshID = actorMeshes[n].handle.id;
                vulkanRenderer->actors[animationActorsCounter].modelMatrix =
                    computeModelMatrix(transformComponent, rotationComponent);
                vulkanRenderer->actors[animationActorsCounter].jointMatrices =
                    updateAnimationFrames(animationComponent, meshID);
                vulkanRenderer->actors[animationActorsCounter].meshID = meshID;
                vulkanRenderer->actors[animationActorsCounter]
                    .diffuseTextureIndex =
                    materialComponent->diffuseTextureID_.id;
                vulkanRenderer->actors[animationActorsCounter]
                    .specularTextureIndex =
                    materialComponent->specularTextureID_.id;
                vulkanRenderer->actors[animationActorsCounter].ambient =
                    materialComponent->ambient;
                vulkanRenderer->actors[animationActorsCounter].shininess =
                    materialComponent->shininess;
                ++animationActorsCounter;
            }
        }
    }

    staticActorsArchetypesNumber = 0;
    world.searchCacheArchetypes(
        staticActorsRequiredMask,
        cachedStaticActorsArchetypes,
        staticActorsArchetypesNumber
    );

    uint32_t staticActorsCounter = animationActorsCounter;
    for (uint32_t x = 0; x < staticActorsArchetypesNumber; ++x) {
        Archetype* arch = cachedStaticActorsArchetypes[x];
        transform* staticActorTransforms =
            (transform*)arch->components[ComponentsIndices::TransformComponent];
        mesh* staticActorMeshes =
            (mesh*)arch->components[ComponentsIndices::MeshComponent];
        material* staticActorMaterials =
            (material*)arch->components[ComponentsIndices::MaterialComponent];
        rotation* staticActorRotations =
            (rotation*)arch->components[ComponentsIndices::RotationComponent];

        std::vector<Matrix<float, 4>> jointMatrices;
        jointMatrices.resize(MAX_JOINTS_NUMBER);
        for (unsigned int i = 0; i < MAX_JOINTS_NUMBER; ++i) {
            Matrix<float, 4> unitMatrix(1.0f);
            jointMatrices[i] = unitMatrix;
        }

        for (uint32_t n = 0; n < arch->entityCount; ++n) {
            vulkanRenderer->actors.push_back({});
            transform* transformComponent = &staticActorTransforms[n];
            material* materialComponent = &staticActorMaterials[n];
            rotation* rotationComponent = &staticActorRotations[n];
            if (staticActorTransforms && staticActorMaterials
                && staticActorRotations && staticActorMeshes) {
                unsigned int meshID = staticActorMeshes[n].handle.id;
                vulkanRenderer->actors[staticActorsCounter].modelMatrix =
                    computeModelMatrix(transformComponent, rotationComponent);
                vulkanRenderer->actors[staticActorsCounter].jointMatrices =
                    jointMatrices;
                vulkanRenderer->actors[staticActorsCounter].meshID = meshID;
                vulkanRenderer->actors[staticActorsCounter].diffuseTextureIndex =
                    materialComponent->diffuseTextureID_.id;
                vulkanRenderer->actors[staticActorsCounter]
                    .specularTextureIndex =
                    materialComponent->specularTextureID_.id;
                vulkanRenderer->actors[staticActorsCounter].ambient =
                    materialComponent->ambient;
                vulkanRenderer->actors[staticActorsCounter].shininess =
                    materialComponent->shininess;
                ++staticActorsCounter;
            }
        }
    }

    projectileActorsArchetypesNumber = 0;
    world.searchCacheArchetypes(
        projectileRequiredMask,
        cachedProjectileActorsArchetypes,
        projectileActorsArchetypesNumber
    );

    uint32_t projectileActorsCounter = staticActorsCounter;
    for (uint32_t x = 0; x < projectileActorsArchetypesNumber; ++x) {
        Archetype* arch = cachedProjectileActorsArchetypes[x];
        transform* actorTransforms =
            (transform*)arch->components[ComponentsIndices::TransformComponent];
        mesh* actorMeshes =
            (mesh*)arch->components[ComponentsIndices::MeshComponent];
        ProjectileBundle* actorProjectileBundles =
            (ProjectileBundle*)
                arch->components[ComponentsIndices::ProjectileBundleComponent];
        rotation* actorRotations =
            (rotation*)arch->components[ComponentsIndices::RotationComponent];

        std::vector<Matrix<float, 4>> jointMatrices;
        jointMatrices.resize(MAX_JOINTS_NUMBER);
        for (unsigned int i = 0; i < MAX_JOINTS_NUMBER; ++i) {
            Matrix<float, 4> unitMatrix(1.0f);
            jointMatrices[i] = unitMatrix;
        }

        for (uint32_t n = 0; n < arch->entityCount; ++n) {
            vulkanRenderer->actors.push_back({});
            transform* transformComponent = &actorTransforms[n];
            material* materialComponent = &actorProjectileBundles[n].material;
            rotation* rotationComponent = &actorRotations[n];
            if (actorTransforms && actorProjectileBundles && actorRotations
                && actorMeshes) {
                unsigned int meshID = actorMeshes[n].handle.id;
                vulkanRenderer->actors[projectileActorsCounter].modelMatrix =
                    computeModelMatrix(transformComponent, rotationComponent);
                vulkanRenderer->actors[projectileActorsCounter].jointMatrices =
                    jointMatrices;
                vulkanRenderer->actors[projectileActorsCounter].meshID = meshID;
                vulkanRenderer->actors[projectileActorsCounter]
                    .diffuseTextureIndex =
                    materialComponent->diffuseTextureID_.id;
                vulkanRenderer->actors[projectileActorsCounter]
                    .specularTextureIndex =
                    materialComponent->specularTextureID_.id;
                vulkanRenderer->actors[projectileActorsCounter].ambient =
                    materialComponent->ambient;
                vulkanRenderer->actors[projectileActorsCounter].shininess =
                    materialComponent->shininess;
                ++projectileActorsCounter;
            }
        }
    }

    // Item actors renders in the game world.

    itemActorsArchetypesNumber = 0;
    world.searchCacheArchetypes(
        rotationItemRequiredMask,
        cachedItemActorsArchetypes,
        itemActorsArchetypesNumber
    );

    uint32_t itemActorsCounter = projectileActorsCounter;
    for (uint32_t x = 0; x < itemActorsArchetypesNumber; ++x) {
        Archetype* arch = cachedItemActorsArchetypes[x];
        transform* itemTransforms =
            (transform*)arch->components[ComponentsIndices::TransformComponent];
        mesh* itemMeshes =
            (mesh*)arch->components[ComponentsIndices::MeshComponent];
        material* itemMaterials =
            (material*)arch->components[ComponentsIndices::MaterialComponent];
        rotation* itemRotations =
            (rotation*)arch->components[ComponentsIndices::RotationComponent];
        item* items = (item*)arch->components[ComponentsIndices::ItemComponent];

        std::vector<Matrix<float, 4>> jointMatrices;
        jointMatrices.resize(MAX_JOINTS_NUMBER);
        for (unsigned int i = 0; i < MAX_JOINTS_NUMBER; ++i) {
            Matrix<float, 4> unitMatrix(1.0f);
            jointMatrices[i] = unitMatrix;
        }

        for (uint32_t n = 0; n < arch->entityCount; ++n) {
            if (items[n].isActor) {
                vulkanRenderer->actors.push_back({});
                transform* transformComponent = &itemTransforms[n];
                material* materialComponent = &itemMaterials[n];
                rotation* rotationComponent = &itemRotations[n];
                if (itemTransforms && itemMaterials && itemRotations
                    && itemMeshes) {
                    unsigned int meshID = itemMeshes[n].handle.id;
                    vulkanRenderer->actors[itemActorsCounter].modelMatrix =
                        computeModelMatrix(
                            transformComponent,
                            rotationComponent
                        );
                    vulkanRenderer->actors[itemActorsCounter].jointMatrices =
                        jointMatrices;
                    vulkanRenderer->actors[itemActorsCounter].meshID = meshID;
                    vulkanRenderer->actors[itemActorsCounter]
                        .diffuseTextureIndex =
                        materialComponent->diffuseTextureID_.id;
                    vulkanRenderer->actors[itemActorsCounter]
                        .specularTextureIndex =
                        materialComponent->specularTextureID_.id;
                    vulkanRenderer->actors[itemActorsCounter].ambient =
                        materialComponent->ambient;
                    vulkanRenderer->actors[itemActorsCounter].shininess =
                        materialComponent->shininess;
                    ++itemActorsCounter;
                }
            }
        }
    }

    vulkanRenderer->players.clear();
    playerArchetypesNumber = 0;
    world.searchCacheArchetypes(
        playerRequiredMask,
        cachedPlayerArchetypes,
        playerArchetypesNumber
    );

    uint32_t playerEntityCount = 0;
    for (uint32_t x = 0; x < playerArchetypesNumber; ++x) {
        Archetype* arch = cachedPlayerArchetypes[x];
        transform* playerTransforms =
            (transform*)arch->components[ComponentsIndices::TransformComponent];

        for (unsigned int n = 0; n < arch->entityCount; ++n) {
            vulkanRenderer->players.push_back({});
            transform* playerTransformComponent = &playerTransforms[n];
            if (&playerTransforms[n] != nullptr) {
                vulkanRenderer->players[playerEntityCount].position =
                    playerTransformComponent->position;
                vulkanRenderer->players[playerEntityCount].forward =
                    playerTransformComponent->forward;
            }
        }
        ++playerEntityCount;
    }
}

void Engine::loadWavefrontObj() {
    for (unsigned int m = 0; m < pathsArray_.size(); ++m) {
        CWaveFrontObjParser parser;
        CWaveFrontObjParser* wavefrontObjParser = &parser;

        wavefrontObjParser->ReadFile(pathsArray_[m]);
        wavefrontObjParser->ParseFile();

        vulkanRenderer->aIndices_.emplace_back();
        vulkanRenderer->aVertices_.emplace_back();
        vulkanRenderer->highest_gltf_Y.emplace_back();
        vulkanRenderer->highest_gltf_Y[m] = -999.999f;

        vulkanRenderer->frames.push_back({});
        vulkanRenderer->jointMatricesPerMesh.push_back({});

        unsigned int vertexIndex = 0;
        unsigned int textureIndex = 0;
        unsigned int normalIndex = 0;
        unsigned int faceVerticesSize = wavefrontObjParser->getFaces().size();
        vulkanRenderer->meshAxisLimitingValues.setToDefaultValues();

        for (unsigned int i = 0; i < faceVerticesSize; ++i) {
            for (int j = 0; j < 3; ++j) {
                vertexIndex = wavefrontObjParser->getFaces()[i][0][j] - 1;
                vulkanRenderer->aIndices_[m].push_back(i * 3 + j);
                SVertex vertex =
                    wavefrontObjParser->getCoordinateVertices()[vertexIndex];
                textureIndex = wavefrontObjParser->getFaces()[i][1][j] - 1;
                SVertex texture =
                    wavefrontObjParser->getTextureVertices()[textureIndex];
                normalIndex = wavefrontObjParser->getFaces()[i][2][j] - 1;
                SVertex normal = wavefrontObjParser->getNormals()[normalIndex];

                Vector<float, 4> jointIndices;
                Vector<float, 4> weights;

                if (vertex[1] > vulkanRenderer->highest_gltf_Y[m]) {
                    vulkanRenderer->highest_gltf_Y[m] = vertex[1];
                }

                if (vertex[0]
                    < vulkanRenderer->meshAxisLimitingValues.lowest_x) {
                    vulkanRenderer->meshAxisLimitingValues.lowest_x = vertex[0];
                } else if (
                    vertex[0] > vulkanRenderer->meshAxisLimitingValues.highest_x
                ) {
                    vulkanRenderer->meshAxisLimitingValues.highest_x =
                        vertex[0];
                }

                if (vertex[1]
                    < vulkanRenderer->meshAxisLimitingValues.lowest_y) {
                    vulkanRenderer->meshAxisLimitingValues.lowest_y = vertex[1];
                } else if (
                    vertex[1] > vulkanRenderer->meshAxisLimitingValues.highest_y
                ) {
                    vulkanRenderer->meshAxisLimitingValues.highest_y =
                        vertex[1];
                }

                if (vertex[2]
                    < vulkanRenderer->meshAxisLimitingValues.lowest_z) {
                    vulkanRenderer->meshAxisLimitingValues.lowest_z = vertex[2];
                } else if (
                    vertex[2] > vulkanRenderer->meshAxisLimitingValues.highest_z
                ) {
                    vulkanRenderer->meshAxisLimitingValues.highest_z =
                        vertex[2];
                }

                jointIndices[0] = -1;
                jointIndices[1] = -1;
                jointIndices[2] = -1;
                jointIndices[3] = -1;

                weights[0] = 1.0f;
                weights[1] = 1.0f;
                weights[2] = 1.0f;
                weights[2] = 1.0f;

                vulkanRenderer->aVertices_[m].push_back(
                    {{vertex[0], vertex[1], vertex[2]},
                     {normal[0], normal[1], normal[2]},
                     {texture[0], texture[1]},
                     {jointIndices[0], jointIndices[1], jointIndices[2]},
                     {weights[0], weights[1], weights[2]}}
                );
            }
        }
        setMeshBounds(vulkanRenderer->meshAxisLimitingValues);
        ++wavefrontObjCounter;
    }
}

void Engine::calculateMeshBounds(const Vector<float, 4>& animated_vertex) {
    if (animated_vertex[0] < vulkanRenderer->meshAxisLimitingValues.lowest_x) {
        vulkanRenderer->meshAxisLimitingValues.lowest_x = animated_vertex[0];
    } else if (
        animated_vertex[0] > vulkanRenderer->meshAxisLimitingValues.highest_x
    ) {
        vulkanRenderer->meshAxisLimitingValues.highest_x = animated_vertex[0];
    }

    if (animated_vertex[1] < vulkanRenderer->meshAxisLimitingValues.lowest_y) {
        vulkanRenderer->meshAxisLimitingValues.lowest_y = animated_vertex[1];
    } else if (
        animated_vertex[1] > vulkanRenderer->meshAxisLimitingValues.highest_y
    ) {
        vulkanRenderer->meshAxisLimitingValues.highest_y = animated_vertex[1];
    }

    if (animated_vertex[2] < vulkanRenderer->meshAxisLimitingValues.lowest_z) {
        vulkanRenderer->meshAxisLimitingValues.lowest_z = animated_vertex[2];
    } else if (
        animated_vertex[2] > vulkanRenderer->meshAxisLimitingValues.highest_z
    ) {
        vulkanRenderer->meshAxisLimitingValues.highest_z = animated_vertex[2];
    }
}

bool Engine::isModelCacheExists(const std::string& modelFilePath) {
    std::ofstream modelsCache(
        "../../../examples/assets/cache/models/cache",
        std::ios::app
    );

    if (!modelsCache.is_open()) {
        std::cerr << "Error opening the models cache file" << std::endl;
        throw std::runtime_error("Failed to load mesh cache");
    }

    std::ifstream file("../../../examples/assets/cache/models/cache");

    std::string line;
    while (std::getline(file, line)) {
        if (line.find(modelFilePath) != std::string::npos) {
            std::cout << "Model with pafile path: " << modelFilePath
                      << " is already exists in cache" << std::endl;

            std::istringstream iss(line);

            std::string keyword;
            float highest_x, lowest_x, highest_y, lowest_y, highest_z, lowest_z;

            iss >> keyword >> highest_x >> lowest_x >> highest_y >> lowest_y
                >> highest_z >> lowest_z;
            vulkanRenderer->meshAxisLimitingValues.highest_x = highest_x;
            vulkanRenderer->meshAxisLimitingValues.lowest_x = lowest_x;
            vulkanRenderer->meshAxisLimitingValues.highest_y = highest_y;
            vulkanRenderer->meshAxisLimitingValues.lowest_y = lowest_y;
            vulkanRenderer->meshAxisLimitingValues.highest_z = highest_z;
            vulkanRenderer->meshAxisLimitingValues.lowest_z = lowest_z;

            isAlreadyCached = true;

            modelsCache.close();
            return true;
        }
    }

    modelsCache.close();
    return false;
}

void Engine::writeModelsCache(const std::string& modelFilePath) {
    std::ofstream modelsCache(
        "../../../examples/assets/cache/models/cache",
        std::ios::app
    );

    if (!modelsCache.is_open()) {
        std::cerr << "Error opening the models cache file" << std::endl;
        throw std::runtime_error("Failed to load mesh cache");
    }

    std::size_t pos = modelFilePath.find(' ');
    std::string firstPart = (pos == std::string::npos)
        ? modelFilePath
        : modelFilePath.substr(0, pos);

    modelsCache << modelFilePath;
    modelsCache << " " << vulkanRenderer->meshAxisLimitingValues.highest_x
                << " " << vulkanRenderer->meshAxisLimitingValues.lowest_x << " "
                << vulkanRenderer->meshAxisLimitingValues.highest_y << " "
                << vulkanRenderer->meshAxisLimitingValues.lowest_y << " "
                << vulkanRenderer->meshAxisLimitingValues.highest_z << " "
                << vulkanRenderer->meshAxisLimitingValues.lowest_z << std::endl;

    modelsCache.close();
}

void Engine::initializeGLTF() {
    std::vector<bool> animationFlags;
    for (unsigned int m = 0; m < pathsGLTF_.size(); ++m) {
        CJsonParser jsonParser;
        vulkanRenderer->aVertexesTemp_.emplace_back();
        vulkanRenderer->aIndices_.emplace_back();
        vulkanRenderer->frames.push_back({});
        vulkanRenderer->jointMatricesPerMesh.push_back({});
        animationFlags.push_back({});
        vulkanRenderer->highest_gltf_Y.emplace_back();
        uint32_t nextIndexGLTF = wavefrontObjCounter + m;
        bool animationFlag = false;
        jsonParser.LoadGLTF(
            pathsGLTF_[m],
            vulkanRenderer->aVertexesTemp_[m],
            vulkanRenderer->aIndices_[nextIndexGLTF],
            vulkanRenderer->jointMatricesPerMesh[nextIndexGLTF],
            vulkanRenderer->frames[nextIndexGLTF],
            animationFlag,
            vulkanRenderer->highest_gltf_Y[nextIndexGLTF]
        );
        animationFlags[m] = animationFlag;
    }

    for (unsigned int m = 0; m < pathsGLTF_.size(); ++m) {
        vulkanRenderer->aVertices_.emplace_back();
        vulkanRenderer->meshAxisLimitingValues.setToDefaultValues();

        isAlreadyCached = false;
        isModelCacheExists(pathsGLTF_[m]);

        int stepOffset = 0;
        if (animationFlags[m]) {
            stepOffset = 8;
        } else {
            stepOffset = 16;
        }

        for (unsigned int n = 0; n < vulkanRenderer->aVertexesTemp_[m].size();
             n += stepOffset) {
            SVertex vertex;
            vertex[0] = vulkanRenderer->aVertexesTemp_[m][n];
            vertex[1] = vulkanRenderer->aVertexesTemp_[m][n + 1];
            vertex[2] = vulkanRenderer->aVertexesTemp_[m][n + 2];
            SVertex normal;
            normal[0] = vulkanRenderer->aVertexesTemp_[m][n + 3];
            normal[1] = vulkanRenderer->aVertexesTemp_[m][n + 4];
            normal[2] = vulkanRenderer->aVertexesTemp_[m][n + 5];
            SVertex texture;
            texture[0] = vulkanRenderer->aVertexesTemp_[m][n + 6];
            texture[1] = vulkanRenderer->aVertexesTemp_[m][n + 7];

            Vector<float, 4> joinIndices;
            Vector<float, 4> weights;
            if (animationFlags[m]) {
                joinIndices[0] = -1;
                joinIndices[1] = -1;
                joinIndices[2] = -1;
                joinIndices[3] = -1;

                weights[0] = 1;
                weights[1] = 1;
                weights[2] = 1;
                weights[3] = 1;

            } else {
                joinIndices[0] = vulkanRenderer->aVertexesTemp_[m][n + 8];
                joinIndices[1] = vulkanRenderer->aVertexesTemp_[m][n + 9];
                joinIndices[2] = vulkanRenderer->aVertexesTemp_[m][n + 10];
                joinIndices[3] = vulkanRenderer->aVertexesTemp_[m][n + 11];

                weights[0] = vulkanRenderer->aVertexesTemp_[m][n + 12];
                weights[1] = vulkanRenderer->aVertexesTemp_[m][n + 13];
                weights[2] = vulkanRenderer->aVertexesTemp_[m][n + 14];
                weights[3] = vulkanRenderer->aVertexesTemp_[m][n + 15];
            }

            uint32_t nextIndexGLTF = wavefrontObjCounter + m;
            vulkanRenderer->aVertices_[nextIndexGLTF].push_back(
                {{vertex[0], vertex[1], vertex[2]},
                 {normal[0], normal[1], normal[2]},
                 {texture[0], texture[1]},
                 {joinIndices[0],
                  joinIndices[1],
                  joinIndices[2],
                  joinIndices[3]},
                 {weights[0], weights[1], weights[2], weights[3]}}
            );

            if (isAlreadyCached) {
                continue;
            }

            Vector<float, 4> animatedVertex =
                Vector<float, 4>(vertex[0], vertex[1], vertex[2], 1.0);
            if (!animationFlags[m]
                && vulkanRenderer->jointMatricesPerMesh[nextIndexGLTF].size()
                    > 0) {
                for (unsigned int frame = 0; frame
                     < vulkanRenderer->jointMatricesPerMesh[nextIndexGLTF][0]
                           .size();
                     ++frame) {
                    Matrix<float, 4> skinMatrix =
                        (vulkanRenderer
                             ->jointMatricesPerMesh[nextIndexGLTF]
                                                   [int(joinIndices[0])][frame]
                         * weights[0])
                        + (vulkanRenderer
                               ->jointMatricesPerMesh[nextIndexGLTF]
                                                     [int(joinIndices[1])][frame]
                           * weights[1])
                        + (vulkanRenderer
                               ->jointMatricesPerMesh[nextIndexGLTF]
                                                     [int(joinIndices[2])][frame]
                           * weights[2])
                        + (vulkanRenderer
                               ->jointMatricesPerMesh[nextIndexGLTF]
                                                     [int(joinIndices[3])][frame]
                           * weights[3]);

                    animatedVertex =
                        Vector<float, 4>(vertex[0], vertex[1], vertex[2], 1.0)
                        * skinMatrix;
                    calculateMeshBounds(animatedVertex);
                }
            } else {
                calculateMeshBounds(animatedVertex);
            }
        }

        if (!isAlreadyCached) {
            writeModelsCache(pathsGLTF_[m]);
        }
        setMeshBounds(vulkanRenderer->meshAxisLimitingValues);
    }
}

void Engine::initializeFontData() {
    constexpr float fontStep = 1.0 / 12;
    constexpr unsigned int glyph_row = 7;
    constexpr unsigned int glyph_column = 12;

    vulkanRenderer->fontVertexBufferContainer.resize(128);
    vulkanRenderer->fontVertexBufferMemoryContainer.resize(128);

    vulkanRenderer->fontIndexBufferContainer.resize(128);
    vulkanRenderer->fontIndexBufferMemoryContaner.resize(128);

    for (unsigned int i = 0; i < glyph_row; ++i) {
        for (unsigned int j = 0; j < glyph_column; ++j) {
            std::vector<Vertex> symbol_g_vertices;
            symbol_g_vertices.push_back(
                {{-0.5f, 0.5f, 0.0f},
                 {0.0f, 1.0f, 0.0f},
                 {fontStep * j, fontStep * i + fontStep},
                 {0.0f, 0.0f, 0.0f, 0.0f},
                 {1.0f, 0.0f, 0.0f, 0.0f}}
            );
            symbol_g_vertices.push_back(
                {{0.5f, 0.5f, 0.0f},
                 {1.0f, 1.0f, 0.0f},
                 {fontStep * j + fontStep, fontStep * i + fontStep},
                 {0.0f, 0.0f, 0.0f, 0.0f},
                 {1.0f, 0.0f, 0.0f, 0.0f}}
            );
            symbol_g_vertices.push_back(
                {{-0.5f, -0.5f, 0.0f},
                 {0.0f, 0.0f, 0.0f},
                 {fontStep * j, fontStep * i},
                 {0.0f, 0.0f, 0.0f, 0.0f},
                 {1.0f, 0.0f, 0.0f, 0.0f}}
            );
            symbol_g_vertices.push_back(
                {{0.5f, -0.5f, 0.0f},
                 {1.0f, 0.0f, 0.0f},
                 {fontStep * j + fontStep, fontStep * i},
                 {0.0f, 0.0f, 0.0f, 0.0f},
                 {1.0f, 0.0f, 0.0f, 0.0f}}
            );
            unsigned int currentBufferIndex = i * glyph_column + j;

            bool exitFlag = false;
            const unsigned int nextBufferIndex =
                static_cast<const unsigned int>(
                    vulkanRenderer->glyphs[currentBufferIndex]
                );
            // TODO: Fix gabage algorithm.
            for (unsigned int n = 0;
                 n < vulkanRenderer->fontIndicesContainer.size();
                 ++n) {
                if (nextBufferIndex
                    == vulkanRenderer->fontIndicesContainer[n]) {
                    exitFlag = true;
                }
            }

            if (exitFlag) {
                continue;
            }

            vulkanRenderer->symbolGVerticesContainer.push_back(
                symbol_g_vertices
            );
            vulkanRenderer->fontIndicesContainer.push_back(nextBufferIndex);
        }
    }
}

Matrix<float, 4> Engine::computeModelMatrix(
    transform* _transformComponent,
    rotation* rotation
) {
    Matrix<float, 4> rotationMatrix(1.0f);
    Matrix<float, 4> scalingMatrix(1.0f);
    Matrix<float, 4> translationMatrix(1.0f);

    scalingMatrix[0][0] = _transformComponent->scale;
    scalingMatrix[1][1] = _transformComponent->scale;
    scalingMatrix[2][2] = _transformComponent->scale;

    translationMatrix[3][0] = _transformComponent->position[0];
    translationMatrix[3][1] = _transformComponent->position[1];
    translationMatrix[3][2] = _transformComponent->position[2];
    translationMatrix[3][3] = 1.0f;

    float sinPitch = std::sin(Radians(-rotation->pitch / 2));
    float cosPitch = std::cos(Radians(-rotation->pitch / 2));
    float sinYaw = std::sin(Radians((rotation->yaw) / 2));
    float cosYaw = std::cos(Radians((rotation->yaw) / 2));

    Quaternion pitchQuat;
    Quaternion yawQuat;
    pitchQuat.w = cosPitch;
    pitchQuat.x = sinPitch;
    pitchQuat.y = 0.0f;
    pitchQuat.z = 0.0f;

    yawQuat.w = cosYaw;
    yawQuat.x = 0.0f;
    yawQuat.y = sinYaw;
    yawQuat.z = 0.0f;
    return scalingMatrix * translationMatrix;
}

void Engine::computeHudScreeenCoordinates() {
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
    hud_screen_y -= g_eEvent.mousePointerPosition.offset_Y
        / (float)vulkanRenderer->Window->height;
    hud_screen_x += g_eEvent.mousePointerPosition.offset_X
        / (float)vulkanRenderer->Window->width;
#else
    if (vulkanRenderer->isInventoryOpened || vulkanRenderer->isCursorReleased) {
        // Cursor is free while the inventory is open or the cursor is released:
        // track its real position instead of the locked-mouse offsets.
        hud_screen_x = 1.0f
            - g_eEvent.mousePointerPosition.position_X
                / ((float)vulkanRenderer->Window->width / 2.0f);
        hud_screen_y =
            -(g_eEvent.mousePointerPosition.position_Y
                  / ((float)vulkanRenderer->Window->height / 2.0f)
              - 1.0f);
    } else {
        hud_screen_y -=
            (previousMouseOffsetY - g_eEvent.mousePointerPosition.offset_Y)
            / (float)vulkanRenderer->Window->height;
        hud_screen_x +=
            (previousMouseOffsetX - g_eEvent.mousePointerPosition.offset_X)
            / (float)vulkanRenderer->Window->width;
    }
    previousMouseOffsetX = g_eEvent.mousePointerPosition.offset_X;
    previousMouseOffsetY = g_eEvent.mousePointerPosition.offset_Y;
#endif

    if (hud_screen_x > 1.0f) {
        hud_screen_x = 1.0f;
    } else if (hud_screen_x < -1.0f) {
        hud_screen_x = -1.0f;
    }

    if (hud_screen_y > 1.0f) {
        hud_screen_y = 1.0f;
    } else if (hud_screen_y < -1.0f) {
        hud_screen_y = -1.0f;
    }
}

TextureHandle Engine::LoadTextureFromFile(const char* path_to_texture) {
    uint32_t textureID = textureVector.size();
    TextureHandle textureHandle;
    textureHandle.id = textureID;
    textureVector.push_back({.path_to_image = path_to_texture});
    textureHandlers.push_back(textureHandle);

    return textureHandle;
}

auto Engine::load_texture_from_address(
    unsigned int iWidth,
    unsigned int iHeight,
    unsigned int dat_length,
    unsigned char* u_iData
) -> TextureHandle {
    uint32_t textureID = textureVector.size();
    TextureHandle textureHandle;
    textureHandle.id = textureID;
    textureVector.push_back(
        {.iWidth_ = iWidth,
         .iHeight_ = iHeight,
         .dat_length_ = dat_length,
         .u_iData_ = u_iData}
    );
    textureHandlers.push_back(textureHandle);

    return textureHandle;
}

MeshHandle Engine::load_mesh_from_obj(const char* _pathToMesh) {
    MeshHandle meshHandle;
    meshHandle.id = meshID;
    pathsArray_.push_back(_pathToMesh);
    meshHandlers.push_back(meshHandle);
    ++meshID;

    return meshHandle;
}

MeshHandle Engine::load_mesh_from_gltf(const char* pathToMesh) {
    MeshHandle meshHandle;
    meshHandle.id = meshID;
    pathsGLTF_.push_back(pathToMesh);
    meshHandlers.push_back(meshHandle);
    ++meshID;

    return meshHandle;
}

MeshHandle Engine::LoadMesh() {
    MeshHandle meshHandle;
    meshHandle.id = meshID;
    meshHandlers.push_back(meshHandle);
    ++meshID;

    return meshHandle;
}

void Engine::GameKill() {
    runningSound = false;
    soundEngine->CloseDevice();

    if (sound_thread.joinable()) {
        sound_thread.join();
    }

    delete soundEngine;
    soundEngine = nullptr;

    delete chrono;
    chrono = nullptr;
    delete collisionSystem;
    collisionSystem = nullptr;
    delete movementSystem;
    movementSystem = nullptr;
    delete physicsSystem;
    physicsSystem = nullptr;
    delete projectileSystem;
    projectileSystem = nullptr;
    delete damageSystem;
    damageSystem = nullptr;
    delete enemySytem;
    enemySytem = nullptr;
    delete itemSystem;
    itemSystem = nullptr;
}
} // namespace glvm

namespace glvm {
EntityManager* EntityManager::instance = nullptr;
std::mutex EntityManager::mutex;

EntityManager::EntityManager() {}

EntityManager::~EntityManager() {}

EntityManager* EntityManager::get_instance() {
    std::lock_guard<std::mutex> lock(mutex);
    if (instance == nullptr) {
        instance = new EntityManager();
    }
    return instance;
}

[[nodiscard]] unsigned int EntityManager::create_entity() {
    unsigned int _Entity_ID;
    // Check out wether or not free ID in removed entities registry.
    if (removed_entity_registry.size() > k_iNull) {
        _Entity_ID = removed_entity_registry.front();
        active_entity_registry.push_back(removed_entity_registry.front());
        removed_entity_registry.erase(removed_entity_registry.begin());
    } else {
        active_entity_registry.push_back(id);
        _Entity_ID = id;
        ++id;
    }
    is_entities_collection_changed = true;
    return _Entity_ID;
}

// Don't need to delete real component in this method. Because systems dont work
// with component without indices for that component in ordered container.
void EntityManager::remove_entity(
    unsigned int& entity_id,
    ComponentManager* component_manager
) {
    component_manager->remove_all_components(entity_id);
    active_entity_registry[entity_id] = k_iUint_Max;
    removed_entity_registry.push_back(entity_id);
    is_entities_collection_changed = true;
}
} // namespace glvm

namespace glvm {
CEvent::CEvent() {}

EEvents& CEvent::GetEvent() {
    return eEvent_;
}

void CEvent::SetEvent(EEvents _eEvent) {
    eEvent_ = _eEvent;
}

void CEvent::SetNextEvent(EEvents _eEvent) {
    nextEvent = _eEvent;
}

EEvents CEvent::GetNextEvent() {
    return nextEvent;
}

void CEvent::SetLastEvent(CStack _Stack) {
    switch (_Stack.Pop()) {
        case glvm::eMOVE_RIGHT:
            SetEvent(glvm::EEvents::eMOVE_RIGHT);
            break;
        case glvm::eMOVE_LEFT:
            SetEvent(glvm::EEvents::eMOVE_LEFT);
            break;
        case glvm::eMOVE_BACKWARD:
            SetEvent(glvm::EEvents::eMOVE_BACKWARD);
            break;
        case glvm::eMOVE_FORWARD:
            SetEvent(glvm::EEvents::eMOVE_FORWARD);
            break;
        case glvm::eMOUSE_LEFT_BUTTON:
            SetEvent(glvm::EEvents::eMOUSE_LEFT_BUTTON);
            break;
        default:
            break;
    }
}
} // namespace glvm

namespace glvm {
std::vector<VkDescriptorSet> descriptorSetsChunks;
std::vector<VkRenderPass> renderPasses;
std::vector<Descriptor> GPUDescriptors;
} // namespace glvm

namespace glvm {
void descriptorSetBuilder() {
    // Counts ds bindings indexes inside ds.
    static unsigned int DS_globalBindingsCounter = 0;
    // Counts host data ds.
    static unsigned int DS_hostNumber = 0;
    // Counts offsets data descriptors.
    static unsigned int globalDescriptorsOffset = 0;

    for (unsigned int dsCounter = 0;
         dsCounter < DescriptorSetDataLink::DESCRIPTOR_CHUNKS_NUMBER;
         ++dsCounter) {
        // Offset for indexing inside descriptorSetsChunks.
        descriptorSetsConfig[dsCounter].descriptorSetOffset = DS_hostNumber;
        DS_hostNumber += descriptorSetsConfig[dsCounter].hostDescriptorNumber;

        for (unsigned int DS_localBindingsCounter = 0; DS_localBindingsCounter
             < descriptorSetsConfig[dsCounter]
                   .actualLinkedDescriptorBindingsNumber;
             ++DS_localBindingsCounter) {
            const uint32_t DS_sumBindingsCounter =
                DS_globalBindingsCounter + DS_localBindingsCounter;
            // Global offset for descriptors inside ds binding.
            descriptorBindingsConfig[DS_sumBindingsCounter]
                .globalDescriptorOffset = globalDescriptorsOffset;

            // Index for ds bindings inside ds.
            descriptorSetsConfig[dsCounter]
                .descriptorsBindingsIDs[DS_localBindingsCounter] =
                DS_sumBindingsCounter;
            if (descriptorBindingsConfig[DS_sumBindingsCounter].vkType
                == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) {
                for (unsigned int descriptorCounter = 0; descriptorCounter
                     < descriptorBindingsConfig[DS_sumBindingsCounter]
                           .shaderDescriptorsNumber;
                     ++descriptorCounter) {
                    GPUDescriptors.push_back({});
                    GPUDescriptors[GPUDescriptors.size() - 1].GPUBuffer =
                        new GPUBuffer;
                    ++globalDescriptorsOffset;
                }
            } else if (
                descriptorBindingsConfig[DS_sumBindingsCounter].vkType
                == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
            ) {
                for (unsigned int descriptorCounter = 0; descriptorCounter
                     < descriptorBindingsConfig[DS_sumBindingsCounter]
                           .shaderDescriptorsNumber;
                     ++descriptorCounter) {
                    GPUDescriptors.push_back({});
                    GPUDescriptors[GPUDescriptors.size() - 1].GPUImage =
                        new VK_Image;
                    ++globalDescriptorsOffset;
                }
            }
        }
        DS_globalBindingsCounter +=
            descriptorSetsConfig[dsCounter].actualLinkedDescriptorBindingsNumber;
    }
    descriptorSetsChunks.resize(DS_hostNumber);
}

void pipelineBuilder() {
    static unsigned int descriptorSetsLayoutIdCounter = 0;
    for (unsigned int pipelineCounter = 0;
         pipelineCounter < SpecificPipeline::PIPELINES_NUMBER;
         ++pipelineCounter) {
        for (unsigned int linkedDSLayoutCounter = 0; linkedDSLayoutCounter
             < pipelineConfigs[pipelineCounter].actualLinkedDescriptorSetsNumber;
             ++linkedDSLayoutCounter) {
            pipelineConfigs[pipelineCounter]
                .linkedDescriptorSetIDs[linkedDSLayoutCounter] =
                descriptorSetsLayoutIdCounter + linkedDSLayoutCounter;
        }
        descriptorSetsLayoutIdCounter +=
            pipelineConfigs[pipelineCounter].actualLinkedDescriptorSetsNumber;
    }
}

void renderPassesBuilder() {
    renderPasses.resize(SpecificPipeline::PIPELINES_NUMBER);
}
}; // namespace glvm

namespace glvm {
VkResult CreateDebugUtilsMessengerEXT(
    VkInstance instance,
    const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkDebugUtilsMessengerEXT* pDebugMessenger
) {
    static auto func = (PFN_vkCreateDebugUtilsMessengerEXT)
        vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void CreateBeginDebugUtilsLabelEXT(
    [[maybe_unused]] VkInstance instance,
    [[maybe_unused]] VkCommandBuffer commandBuffer,
    [[maybe_unused]] const VkDebugUtilsLabelEXT* labelInfo
) {
#ifndef NDEBUG
    static auto func = (PFN_vkCmdBeginDebugUtilsLabelEXT)
        vkGetInstanceProcAddr(instance, "vkCmdBeginDebugUtilsLabelEXT");
    func(commandBuffer, labelInfo);
#endif
}

void CreateEndDebugUtilsLabelEXT(
    [[maybe_unused]] VkInstance instance,
    [[maybe_unused]] VkCommandBuffer commandBuffer
) {
#ifndef NDEBUG
    static auto func = (PFN_vkCmdEndDebugUtilsLabelEXT)
        vkGetInstanceProcAddr(instance, "vkCmdEndDebugUtilsLabelEXT");
    func(commandBuffer);
#endif
}

void DestroyDebugUtilsMessengerEXT(
    VkInstance instance,
    VkDebugUtilsMessengerEXT debugMessenger,
    const VkAllocationCallbacks* pAllocator
) {
    static auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)
        vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}

VkResult SetDebugObjectName(
    VkDevice device,
    const VkDebugUtilsObjectNameInfoEXT* objectNameInfo
) {
    static auto func = (PFN_vkSetDebugUtilsObjectNameEXT)
        vkGetDeviceProcAddr(device, "vkSetDebugUtilsObjectNameEXT");
    if (func != nullptr) {
        return func(device, objectNameInfo);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void setImageDebugObjectName(
    VkDevice device,
    VK_Image image,
    std::string imageName
) {
    VkDebugUtilsObjectNameInfoEXT imageObjectInfo {};
    imageObjectInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    std::string imageName1 = std::string(VK_DEBUG_IMAGE_SET_RED) + " \x1b[31m"
        + imageName + " pipeline #\x1b[0m " + std::to_string(0);
    const char* strImageName = imageName1.c_str();
    imageObjectInfo.pObjectName = strImageName;
    imageObjectInfo.objectType = VK_OBJECT_TYPE_IMAGE;
    imageObjectInfo.objectHandle = (uint64_t)image.image;
    SetDebugObjectName(device, &imageObjectInfo);
}

void setPipelineDebugObjectName(
    VkDevice device,
    VkPipeline pipeline,
    std::string pipelineName
) {
    VkDebugUtilsObjectNameInfoEXT mainPipelineObjectInfo {};
    mainPipelineObjectInfo.sType =
        VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    std::string mainPipeLineImageName = std::string(VK_DEBUG_PIPELINE_RED)
        + " \x1b[31m" + pipelineName + " pipeline #\x1b[0m "
        + std::to_string(0);
    const char* mainPipeLineStrImageName = mainPipeLineImageName.c_str();
    mainPipelineObjectInfo.pObjectName = mainPipeLineStrImageName;
    mainPipelineObjectInfo.objectType = VK_OBJECT_TYPE_PIPELINE;
    mainPipelineObjectInfo.objectHandle = (uint64_t)pipeline;
    SetDebugObjectName(device, &mainPipelineObjectInfo);
}

void setDescriptorSetObjectName(
    VkDevice device,
    VkDescriptorSet descriptorSet,
    std::string descriptorSetName,
    unsigned int index
) {
    VkDebugUtilsObjectNameInfoEXT descriptorSetObjectInfo {};
    descriptorSetObjectInfo.sType =
        VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    std::string name = std::string(VK_DEBUG_DESCRIPTOR_SET_RED) + " \x1b[31m"
        + descriptorSetName + " descriptor set #\x1b[0m "
        + std::to_string(index);
    const char* strName = name.c_str();
    descriptorSetObjectInfo.pObjectName = strName;
    descriptorSetObjectInfo.objectType = VK_OBJECT_TYPE_DESCRIPTOR_SET;
    descriptorSetObjectInfo.objectHandle = (uint64_t)descriptorSet;
    SetDebugObjectName(device, &descriptorSetObjectInfo);
}

void setDebugObjectNames(
    VkDevice device,
    const std::vector<VkBuffer>& vertexBufferContainer,
    const std::vector<VkBuffer>& indexBufferContainer,
    const std::vector<Descriptor>& GPUDescriptors,
    const std::vector<unsigned int>& fontIndicesContainer,
    const std::vector<VkBuffer>& fontVertexBufferContainer,
    const std::vector<VkBuffer>& fontIndexBufferContainer
) {
    setPipelineDebugObjectName(
        device,
        pipelineConfigs[SpecificPipeline::FONT_PIPELINE].pipeline,
        "fontPipeline"
    );
    setPipelineDebugObjectName(
        device,
        pipelineConfigs[SpecificPipeline::UI_PIPELINE].pipeline,
        "uiPipeline"
    );
    setPipelineDebugObjectName(
        device,
        pipelineConfigs[SpecificPipeline::UI_ICONS_PIPELINE].pipeline,
        "uiIconsPipeline"
    );

    VkDebugUtilsObjectNameInfoEXT mainPipelineObjectInfo {};
    mainPipelineObjectInfo.sType =
        VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    std::string mainPipeLineImageName = std::string(VK_DEBUG_PIPELINE_RED)
        + " \x1b[31mMain pipeline #\x1b[0m " + std::to_string(0);
    const char* mainPipeLineStrImageName = mainPipeLineImageName.c_str();
    mainPipelineObjectInfo.pObjectName = mainPipeLineStrImageName;
    mainPipelineObjectInfo.objectType = VK_OBJECT_TYPE_PIPELINE;
    mainPipelineObjectInfo.objectHandle =
        (uint64_t)pipelineConfigs[SpecificPipeline::MAIN_RENDER_PIPELINE]
            .pipeline;
    SetDebugObjectName(device, &mainPipelineObjectInfo);

    VkDebugUtilsObjectNameInfoEXT mainPipelineLayoutObjectInfo {};
    mainPipelineLayoutObjectInfo.sType =
        VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    std::string mainPipelineLayoutImageName =
        std::string(VK_DEBUG_PIPELINE_LAYOUT_RED)
        + " \x1b[31mMain pipeline layout #\x1b[0m " + std::to_string(0);
    const char* mainPipelineLayoutStrImageName =
        mainPipelineLayoutImageName.c_str();
    mainPipelineLayoutObjectInfo.pObjectName = mainPipelineLayoutStrImageName;
    mainPipelineLayoutObjectInfo.objectType = VK_OBJECT_TYPE_PIPELINE_LAYOUT;
    mainPipelineLayoutObjectInfo.objectHandle =
        (uint64_t)pipelineConfigs[SpecificPipeline::MAIN_RENDER_PIPELINE]
            .pipelineLayout;
    SetDebugObjectName(device, &mainPipelineObjectInfo);

    VkDebugUtilsObjectNameInfoEXT directionalLightPipelineObjectInfo {};
    directionalLightPipelineObjectInfo.sType =
        VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    std::string directionalLightPipeLineImageName =
        std::string(VK_DEBUG_PIPELINE_RED)
        + " \x1b[31mDirectional light pipeline #\x1b[0m " + std::to_string(0);
    const char* directionalLightPipeLineStrImageName =
        directionalLightPipeLineImageName.c_str();
    directionalLightPipelineObjectInfo.pObjectName =
        directionalLightPipeLineStrImageName;
    directionalLightPipelineObjectInfo.objectType = VK_OBJECT_TYPE_PIPELINE;
    directionalLightPipelineObjectInfo.objectHandle =
        (uint64_t)pipelineConfigs[SpecificPipeline::DIRECTIONAL_LIGHT_PIPELINE]
            .pipeline;
    SetDebugObjectName(device, &directionalLightPipelineObjectInfo);

    VkDebugUtilsObjectNameInfoEXT directionalLightPipelineLayoutObjectInfo {};
    directionalLightPipelineLayoutObjectInfo.sType =
        VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    std::string directionalLightPipelineLayoutImageName =
        std::string(VK_DEBUG_PIPELINE_LAYOUT_RED)
        + " \x1b[31mDirectional light pipeline layout #\x1b[0m "
        + std::to_string(0);
    const char* directionalLightPipelineLayoutStrImageName =
        directionalLightPipelineLayoutImageName.c_str();
    directionalLightPipelineLayoutObjectInfo.pObjectName =
        directionalLightPipelineLayoutStrImageName;
    directionalLightPipelineLayoutObjectInfo.objectType =
        VK_OBJECT_TYPE_PIPELINE_LAYOUT;
    directionalLightPipelineLayoutObjectInfo.objectHandle =
        (uint64_t)pipelineConfigs[SpecificPipeline::DIRECTIONAL_LIGHT_PIPELINE]
            .pipelineLayout;
    SetDebugObjectName(device, &directionalLightPipelineLayoutObjectInfo);

    VkDebugUtilsObjectNameInfoEXT spotLightPipelineObjectInfo {};
    spotLightPipelineObjectInfo.sType =
        VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    std::string spotLightPipeLineImageName = std::string(VK_DEBUG_PIPELINE_RED)
        + " \x1b[31mSpot light pipeline #\x1b[0m " + std::to_string(0);
    const char* spotLightPipeLineStrImageName =
        spotLightPipeLineImageName.c_str();
    spotLightPipelineObjectInfo.pObjectName = spotLightPipeLineStrImageName;
    spotLightPipelineObjectInfo.objectType = VK_OBJECT_TYPE_PIPELINE;
    spotLightPipelineObjectInfo.objectHandle =
        (uint64_t)pipelineConfigs[SpecificPipeline::SPOT_LIGHT_PIPELINE]
            .pipeline;
    SetDebugObjectName(device, &spotLightPipelineObjectInfo);

    VkDebugUtilsObjectNameInfoEXT spotLightPipelineLayoutObjectInfo {};
    spotLightPipelineLayoutObjectInfo.sType =
        VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    std::string spotLightPipelineLayoutImageName =
        std::string(VK_DEBUG_PIPELINE_LAYOUT_RED)
        + " \x1b[31mSpot light pipeline layout #\x1b[0m " + std::to_string(0);
    const char* spotLightPipelineLayoutStrImageName =
        spotLightPipelineLayoutImageName.c_str();
    spotLightPipelineLayoutObjectInfo.pObjectName =
        spotLightPipelineLayoutStrImageName;
    spotLightPipelineLayoutObjectInfo.objectType =
        VK_OBJECT_TYPE_PIPELINE_LAYOUT;
    spotLightPipelineLayoutObjectInfo.objectHandle =
        (uint64_t)pipelineConfigs[SpecificPipeline::SPOT_LIGHT_PIPELINE]
            .pipelineLayout;
    SetDebugObjectName(device, &spotLightPipelineLayoutObjectInfo);

    VkDebugUtilsObjectNameInfoEXT pointLightPipelineObjectInfo {};
    pointLightPipelineObjectInfo.sType =
        VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    std::string pointLightPipeLineImageName = std::string(VK_DEBUG_PIPELINE_RED)
        + " \x1b[31mPoint light pipeline #\x1b[0m " + std::to_string(0);
    const char* pointLightPipeLineStrImageName =
        pointLightPipeLineImageName.c_str();
    pointLightPipelineObjectInfo.pObjectName = pointLightPipeLineStrImageName;
    pointLightPipelineObjectInfo.objectType = VK_OBJECT_TYPE_PIPELINE;
    pointLightPipelineObjectInfo.objectHandle =
        (uint64_t)pipelineConfigs[SpecificPipeline::POINT_LIGHT_PIPELINE]
            .pipeline;
    SetDebugObjectName(device, &pointLightPipelineObjectInfo);

    VkDebugUtilsObjectNameInfoEXT pointLightPipelineLayoutObjectInfo {};
    pointLightPipelineLayoutObjectInfo.sType =
        VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    std::string pointLightPipelineLayoutImageName =
        std::string(VK_DEBUG_PIPELINE_LAYOUT_RED)
        + " \x1b[31mPoint light pipeline layout #\x1b[0m " + std::to_string(0);
    const char* pointLightPipelineLayoutStrImageName =
        pointLightPipelineLayoutImageName.c_str();
    pointLightPipelineLayoutObjectInfo.pObjectName =
        pointLightPipelineLayoutStrImageName;
    pointLightPipelineLayoutObjectInfo.objectType =
        VK_OBJECT_TYPE_PIPELINE_LAYOUT;
    pointLightPipelineLayoutObjectInfo.objectHandle =
        (uint64_t)pipelineConfigs[SpecificPipeline::POINT_LIGHT_PIPELINE]
            .pipelineLayout;
    SetDebugObjectName(device, &pointLightPipelineLayoutObjectInfo);

    VkDebugUtilsObjectNameInfoEXT hudUniformBufferObjectInfo {};
    hudUniformBufferObjectInfo.sType =
        VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    std::string hudImageName = std::string(VK_DEBUG_IMAGE_SET_RED)
        + " Hud uniform buffer # " + std::to_string(0);
    const char* hudStrImageName = hudImageName.c_str();
    hudUniformBufferObjectInfo.pObjectName = hudStrImageName;
    hudUniformBufferObjectInfo.objectType = VK_OBJECT_TYPE_BUFFER;
    unsigned int hudUboDescriptorBindingIndex =
        descriptorSetsConfig[DescriptorSetDataLink::HUD]
            .descriptorsBindingsIDs[0];
    hudUniformBufferObjectInfo.objectHandle =
        (uint64_t)
            GPUDescriptors[descriptorBindingsConfig[hudUboDescriptorBindingIndex]
                               .globalDescriptorOffset]
                .GPUBuffer->buffer;
    SetDebugObjectName(device, &hudUniformBufferObjectInfo);

    VkDebugUtilsObjectNameInfoEXT fontUniformBufferObjectInfo {};
    fontUniformBufferObjectInfo.sType =
        VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    std::string fontImageName = std::string(VK_DEBUG_IMAGE_SET_RED)
        + " Font uniform buffer # " + std::to_string(0);
    const char* fontStrImageName = fontImageName.c_str();
    fontUniformBufferObjectInfo.pObjectName = fontStrImageName;
    fontUniformBufferObjectInfo.objectType = VK_OBJECT_TYPE_BUFFER;
    unsigned int fontUboDescriptorBindingIndex =
        descriptorSetsConfig[DescriptorSetDataLink::FONT_RENDER_UBO]
            .descriptorsBindingsIDs[0];
    fontUniformBufferObjectInfo.objectHandle =
        (uint64_t)
            GPUDescriptors[descriptorBindingsConfig[fontUboDescriptorBindingIndex]
                               .globalDescriptorOffset]
                .GPUBuffer->buffer;
    SetDebugObjectName(device, &fontUniformBufferObjectInfo);
    VkDebugUtilsObjectNameInfoEXT uiUniformBufferObjectInfo {};
    uiUniformBufferObjectInfo.sType =
        VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    std::string uiImageName = std::string(VK_DEBUG_IMAGE_SET_RED)
        + " UI uniform buffer # " + std::to_string(0);
    const char* uiStrImageName = uiImageName.c_str();
    uiUniformBufferObjectInfo.pObjectName = uiStrImageName;
    uiUniformBufferObjectInfo.objectType = VK_OBJECT_TYPE_BUFFER;
    unsigned int uiUboDescriptorBindingIndex =
        descriptorSetsConfig[DescriptorSetDataLink::UI]
            .descriptorsBindingsIDs[0];
    uiUniformBufferObjectInfo.objectHandle =
        (uint64_t)
            GPUDescriptors[descriptorBindingsConfig[uiUboDescriptorBindingIndex]
                               .globalDescriptorOffset]
                .GPUBuffer->buffer;
    SetDebugObjectName(device, &uiUniformBufferObjectInfo);
    VkDebugUtilsObjectNameInfoEXT uiIconsUniformBufferObjectInfo {};
    uiIconsUniformBufferObjectInfo.sType =
        VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    std::string uiIconsImageName = std::string(VK_DEBUG_IMAGE_SET_RED)
        + " UI icons uniform buffer # " + std::to_string(0);
    const char* uiIconsStrImageName = uiIconsImageName.c_str();
    uiIconsUniformBufferObjectInfo.pObjectName = uiIconsStrImageName;
    uiIconsUniformBufferObjectInfo.objectType = VK_OBJECT_TYPE_BUFFER;
    unsigned int uiIconsUboDescriptorBindingIndex =
        descriptorSetsConfig[DescriptorSetDataLink::UI_ICONS]
            .descriptorsBindingsIDs[0];
    uiIconsUniformBufferObjectInfo.objectHandle =
        (uint64_t)GPUDescriptors
            [descriptorBindingsConfig[uiIconsUboDescriptorBindingIndex]
                 .globalDescriptorOffset]
                .GPUBuffer->buffer;
    SetDebugObjectName(device, &uiIconsUniformBufferObjectInfo);
    VkDebugUtilsObjectNameInfoEXT directionalLightUniformBufferObjectInfo {};
    directionalLightUniformBufferObjectInfo.sType =
        VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    std::string directionalLightImageName = std::string(VK_DEBUG_IMAGE_SET_RED)
        + " Shadow map directional light model matrix uniform buffer # "
        + std::to_string(0);
    const char* directionalLightStrImageName =
        directionalLightImageName.c_str();
    directionalLightUniformBufferObjectInfo.pObjectName =
        directionalLightStrImageName;
    directionalLightUniformBufferObjectInfo.objectType = VK_OBJECT_TYPE_BUFFER;
    unsigned int shadowMapDirectionalLightDescriptorBindingIndex =
        descriptorSetsConfig[DescriptorSetDataLink::SHADOW_MAP_DIRECTIONAL_LIGHT]
            .descriptorsBindingsIDs[0];
    directionalLightUniformBufferObjectInfo.objectHandle =
        (uint64_t)
            GPUDescriptors[descriptorBindingsConfig
                               [shadowMapDirectionalLightDescriptorBindingIndex]
                                   .globalDescriptorOffset]
                .GPUBuffer->buffer;
    SetDebugObjectName(device, &directionalLightUniformBufferObjectInfo);
    VkDebugUtilsObjectNameInfoEXT pointLightUniformBufferObjectInfo {};
    pointLightUniformBufferObjectInfo.sType =
        VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    std::string pointLightImageName = std::string(VK_DEBUG_IMAGE_SET_RED)
        + " Shadow map point light model matrix uniform buffer # "
        + std::to_string(0);
    const char* pointLightStrImageName = pointLightImageName.c_str();
    pointLightUniformBufferObjectInfo.pObjectName = pointLightStrImageName;
    pointLightUniformBufferObjectInfo.objectType = VK_OBJECT_TYPE_BUFFER;
    unsigned int shadowMapPointLightDescriptorBindingIndex =
        descriptorSetsConfig[DescriptorSetDataLink::SHADOW_MAP_POINT_LIGHT]
            .descriptorsBindingsIDs[0];
    pointLightUniformBufferObjectInfo.objectHandle =
        (uint64_t)GPUDescriptors
            [descriptorBindingsConfig[shadowMapPointLightDescriptorBindingIndex]
                 .globalDescriptorOffset]
                .GPUBuffer->buffer;
    SetDebugObjectName(device, &pointLightUniformBufferObjectInfo);
    VkDebugUtilsObjectNameInfoEXT spotLightUniformBufferObjectInfo {};
    spotLightUniformBufferObjectInfo.sType =
        VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    std::string spotLightImageName = std::string(VK_DEBUG_IMAGE_SET_RED)
        + " Shadow map spot light model matrix uniform buffer # "
        + std::to_string(0);
    const char* spotLightStrImageName = spotLightImageName.c_str();
    spotLightUniformBufferObjectInfo.pObjectName = spotLightStrImageName;
    spotLightUniformBufferObjectInfo.objectType = VK_OBJECT_TYPE_BUFFER;
    unsigned int shadowMapSpotLightDescriptorBindingIndex =
        descriptorSetsConfig[DescriptorSetDataLink::SHADOW_MAP_SPOT_LIGHT]
            .descriptorsBindingsIDs[0];
    spotLightUniformBufferObjectInfo.objectHandle =
        (uint64_t)GPUDescriptors
            [descriptorBindingsConfig[shadowMapSpotLightDescriptorBindingIndex]
                 .globalDescriptorOffset]
                .GPUBuffer->buffer;
    SetDebugObjectName(device, &spotLightUniformBufferObjectInfo);
    for (unsigned long i = 0; i < vertexBufferContainer.size(); ++i) {
        VkDebugUtilsObjectNameInfoEXT uniformBufferObjectInfo {};
        uniformBufferObjectInfo.sType =
            VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
        std::string imageName = std::string(VK_DEBUG_IMAGE_SET_RED)
            + " Vertex uniform buffer # " + std::to_string(i);
        const char* strImageName = imageName.c_str();
        uniformBufferObjectInfo.pObjectName = strImageName;
        uniformBufferObjectInfo.objectType = VK_OBJECT_TYPE_BUFFER;
        uniformBufferObjectInfo.objectHandle =
            (uint64_t)vertexBufferContainer[i];
        SetDebugObjectName(device, &uniformBufferObjectInfo);
    }
    for (unsigned long i = 0; i < indexBufferContainer.size(); ++i) {
        VkDebugUtilsObjectNameInfoEXT uniformBufferObjectInfo {};
        uniformBufferObjectInfo.sType =
            VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
        std::string imageName = std::string(VK_DEBUG_IMAGE_SET_RED)
            + " Index uniform buffer # " + std::to_string(i);
        const char* strImageName = imageName.c_str();
        uniformBufferObjectInfo.pObjectName = strImageName;
        uniformBufferObjectInfo.objectType = VK_OBJECT_TYPE_BUFFER;
        uniformBufferObjectInfo.objectHandle =
            (uint64_t)indexBufferContainer[i];
        SetDebugObjectName(device, &uniformBufferObjectInfo);
    }
    for (unsigned long i = 0; i < fontIndicesContainer.size(); ++i) {
        VkDebugUtilsObjectNameInfoEXT uniformBufferObjectInfo {};
        uniformBufferObjectInfo.sType =
            VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
        std::string imageName = std::string(VK_DEBUG_IMAGE_SET_RED)
            + " Font vertex uniform buffer # "
            + std::to_string(fontIndicesContainer[i]);
        const char* strImageName = imageName.c_str();
        uniformBufferObjectInfo.pObjectName = strImageName;
        uniformBufferObjectInfo.objectType = VK_OBJECT_TYPE_BUFFER;
        uniformBufferObjectInfo.objectHandle =
            (uint64_t)fontVertexBufferContainer[fontIndicesContainer[i]];
        SetDebugObjectName(device, &uniformBufferObjectInfo);
    }
    for (unsigned long i = 0; i < fontIndicesContainer.size(); ++i) {
        VkDebugUtilsObjectNameInfoEXT uniformBufferObjectInfo {};
        uniformBufferObjectInfo.sType =
            VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
        std::string imageName = std::string(VK_DEBUG_IMAGE_SET_RED)
            + " Font index uniform buffer # "
            + std::to_string(fontIndicesContainer[i]);
        const char* strImageName = imageName.c_str();
        uniformBufferObjectInfo.pObjectName = strImageName;
        uniformBufferObjectInfo.objectType = VK_OBJECT_TYPE_BUFFER;
        uniformBufferObjectInfo.objectHandle =
            (uint64_t)fontIndexBufferContainer[fontIndicesContainer[i]];
        SetDebugObjectName(device, &uniformBufferObjectInfo);
    }
    VkDebugUtilsObjectNameInfoEXT uniformBufferObjectInfo {};
    uniformBufferObjectInfo.sType =
        VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    std::string imageName = std::string(VK_DEBUG_IMAGE_SET_RED)
        + " Model matrix uniform buffer # " + std::to_string(0);
    const char* strImageName = imageName.c_str();
    uniformBufferObjectInfo.pObjectName = strImageName;
    uniformBufferObjectInfo.objectType = VK_OBJECT_TYPE_BUFFER;
    uniformBufferObjectInfo.objectHandle =
        (uint64_t)GPUDescriptors[DescriptorSetDataLink::MAIN_RENDER_MATRIX_UBO]
            .GPUBuffer->buffer;
    SetDebugObjectName(device, &uniformBufferObjectInfo);
    VkDebugUtilsObjectNameInfoEXT lightDataUniformBufferObjectInfo {};
    lightDataUniformBufferObjectInfo.sType =
        VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    std::string lightDataImageName = std::string(VK_DEBUG_IMAGE_SET_RED)
        + " Light data uniform buffer # " + std::to_string(0);
    const char* lightDataStrImageName = lightDataImageName.c_str();
    lightDataUniformBufferObjectInfo.pObjectName = lightDataStrImageName;
    lightDataUniformBufferObjectInfo.objectType = VK_OBJECT_TYPE_BUFFER;
    unsigned int lightDataUboDescriptorBindingIndex =
        descriptorSetsConfig[DescriptorSetDataLink::MAIN_RENDER_LIGHT_DATA_UBO]
            .descriptorsBindingsIDs[0];
    lightDataUniformBufferObjectInfo.objectHandle =
        (uint64_t)GPUDescriptors
            [descriptorBindingsConfig[lightDataUboDescriptorBindingIndex]
                 .globalDescriptorOffset]
                .GPUBuffer->buffer;
    SetDebugObjectName(device, &lightDataUniformBufferObjectInfo);
}
}; // namespace glvm

namespace glvm {
namespace {
// 16k verts, shared line + quad buffer.
constexpr uint32_t kMaxDebugVertices = 1 << 14;

void pushLine(
    std::vector<DebugVertex>& out,
    const Vector<float, 3>& a,
    const Vector<float, 3>& b,
    const Vector<float, 3>& color
) {
    out.push_back({a[0], a[1], a[2], color[0], color[1], color[2]});
    out.push_back({b[0], b[1], b[2], color[0], color[1], color[2]});
}

void pushBox(
    std::vector<DebugVertex>& out,
    const Vector<float, 3> corners[8],
    const Vector<float, 3>& color
) {
    static const unsigned int edges[12][2] = {
        {0, 1},
        {1, 2},
        {2, 3},
        {3, 0},
        {4, 5},
        {5, 6},
        {6, 7},
        {7, 4},
        {0, 4},
        {1, 5},
        {2, 6},
        {3, 7}
    };
    for (const auto& edge : edges) {
        pushLine(out, corners[edge[0]], corners[edge[1]], color);
    }
}

Vector<float, 4> toVec4(const Vector<float, 3>& v, float w) {
    return Vector<float, 4>(v[0], v[1], v[2], w);
}

Vector<float, 3> fromVec4(const Vector<float, 4>& v) {
    return Vector<float, 3>(v[0], v[1], v[2]);
}
} // namespace

ImGuiOverlay::ImGuiOverlay(CVulkanRenderer& renderer) : renderer_(renderer) {}

bool ImGuiOverlay::wantsMouse() const {
    if (!initialized_) {
        return false;
    }
    ImGuiIO& io = ImGui::GetIO();
    return io.WantCaptureMouse || io.WantCaptureKeyboard;
}

void ImGuiOverlay::init() {
    if (initialized_) {
        return;
    }
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;
    createRenderPass();
    createVertexBuffer();
    createLinePipeline();
    ImGui_ImplWin32_Init(renderer_.Window->GetModernWindowHWND());
    ImGui_ImplVulkan_InitInfo initInfo {};
    initInfo.ApiVersion = VK_API_VERSION_1_0;
    initInfo.Instance = renderer_.instance;
    initInfo.PhysicalDevice = renderer_.physicalDevice;
    initInfo.Device = renderer_.device;
    initInfo.QueueFamily = renderer_.findQueueFamilies(renderer_.physicalDevice)
                               .graphics_family.value();
    initInfo.Queue = renderer_.graphicsQueue;
    initInfo.DescriptorPoolSize = 512;
    initInfo.MinImageCount = MAX_FRAMES_IN_FLIGHT;
    initInfo.ImageCount =
        static_cast<uint32_t>(renderer_.swapChainImages.size());
    initInfo.PipelineInfoMain.RenderPass = renderPass_;
    initInfo.PipelineInfoMain.Subpass = 0;
    if (!ImGui_ImplVulkan_Init(&initInfo)) {
        throw std::runtime_error("failed to init ImGui vulkan backend!");
    }
    initialized_ = true;
    createSwapChainResources();
}

void ImGuiOverlay::shutdown() {
    if (!initialized_) {
        return;
    }
    vkDeviceWaitIdle(renderer_.device);
    destroySwapChainResources();
    vkDestroyBuffer(renderer_.device, vertexBuffer_, nullptr);
    vkFreeMemory(renderer_.device, vertexBufferMemory_, nullptr);
    vertexBuffer_ = VK_NULL_HANDLE;
    vertexBufferMapped_ = nullptr;
    vkDestroyPipeline(renderer_.device, linePipeline_, nullptr);
    vkDestroyPipelineLayout(renderer_.device, lineLayout_, nullptr);
    vkDestroyRenderPass(renderer_.device, renderPass_, nullptr);
    renderPass_ = VK_NULL_HANDLE;
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    initialized_ = false;
}

void ImGuiOverlay::createSwapChainResources() {
    if (!initialized_) {
        return;
    }
    framebuffers_.resize(renderer_.swapChainImageViews.size());
    for (size_t i = 0; i < framebuffers_.size(); ++i) {
        VkImageView attachment = renderer_.swapChainImageViews[i];
        VkFramebufferCreateInfo info {};
        info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        info.renderPass = renderPass_;
        info.attachmentCount = 1;
        info.pAttachments = &attachment;
        info.width = renderer_.swapChainExtent.width;
        info.height = renderer_.swapChainExtent.height;
        info.layers = 1;
        if (vkCreateFramebuffer(
                renderer_.device,
                &info,
                nullptr,
                &framebuffers_[i]
            )
            != VK_SUCCESS) {
            throw std::runtime_error(
                "failed to create imgui overlay framebuffer!"
            );
        }
    }
}

void ImGuiOverlay::destroySwapChainResources() {
    for (VkFramebuffer& framebuffer : framebuffers_) {
        vkDestroyFramebuffer(renderer_.device, framebuffer, nullptr);
    }
    framebuffers_.clear();
}

void ImGuiOverlay::newFrame() {
    if (!initialized_) {
        return;
    }
    ImGui_ImplWin32_NewFrame();
    ImGui_ImplVulkan_NewFrame();
    ImGui::NewFrame();

    if (ImGui::IsKeyPressed(ImGuiKey_F1, false)) {
        showPanel = !showPanel;
    }
    buildPanel();
    buildDebugVertices();
    ImGui::Render();
}

void ImGuiOverlay::recordCommandBuffer(
    VkCommandBuffer commandBuffer,
    uint32_t imageIndex
) {
    if (!initialized_) {
        return;
    }
    VkRenderPassBeginInfo renderPassInfo {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderPass_;
    renderPassInfo.framebuffer = framebuffers_[imageIndex];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = renderer_.swapChainExtent;
    renderPassInfo.clearValueCount = 0;
    renderPassInfo.pClearValues = nullptr;
    vkCmdBeginRenderPass(
        commandBuffer,
        &renderPassInfo,
        VK_SUBPASS_CONTENTS_INLINE
    );
    VkViewport viewport {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(renderer_.swapChainExtent.width);
    viewport.height = static_cast<float>(renderer_.swapChainExtent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
    VkRect2D scissor {};
    scissor.offset = {0, 0};
    scissor.extent = renderer_.swapChainExtent;
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
    if (lineVertexCount_ > 0) {
        vkCmdBindPipeline(
            commandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            linePipeline_
        );
        VkDeviceSize offset = 0;
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBuffer_, &offset);
        Matrix<float, 4> viewProj =
            renderer_.viewMatrix * renderer_.projectionMatrix;
        vkCmdPushConstants(
            commandBuffer,
            lineLayout_,
            VK_SHADER_STAGE_VERTEX_BIT,
            0,
            sizeof(Matrix<float, 4>),
            &viewProj
        );
        vkCmdDraw(commandBuffer, lineVertexCount_, 1, 0, 0);
    }
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
    vkCmdEndRenderPass(commandBuffer);
}

void ImGuiOverlay::buildPanel() {
    if (!showPanel) {
        return;
    }
    ImGui::Begin("Debug overlay", nullptr, ImGuiWindowFlags_NoCollapse);
    ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
    ImGui::Separator();
    ImGui::Checkbox("Actor bounds", &showActorBounds);
    ImGui::Checkbox("Light frustums", &showLightFrustums);
    ImGui::Checkbox("Spatial grid", &showSpatialGrid);
    ImGui::Checkbox("Shadows", &shadowsEnabled);
    ImGui::Checkbox("Shadow maps", &showShadowMaps);
    if (showShadowMaps) {
        ImGui::RadioButton("Directional", &shadowMapMode, 0);
        ImGui::SameLine();
        ImGui::RadioButton("Spot", &shadowMapMode, 1);
        int maxLight = (shadowMapMode == 0)
            ? static_cast<int>(renderer_.directionalLightNumber)
            : static_cast<int>(renderer_.spotLightNumber);
        ImGui::SliderInt("Light", &shadowMapLight, 0, std::max(0, maxLight - 1));
        ImGui::Text(
            "Shadow map #%d of %d",
            shadowMapLight,
            std::max(1, maxLight)
        );
    }
    ImGui::Text("Actors: %zu", renderer_.actors.size());
    ImGui::Text(
        "Dir lights: %u, Spot lights: %u",
        renderer_.directionalLightNumber,
        renderer_.spotLightNumber
    );
    ImGui::Separator();
    if (ImGui::Button("Hide panel (F1)")) {
        showPanel = false;
    }
    ImGui::End();
}

void ImGuiOverlay::buildDebugVertices() {
    std::vector<DebugVertex> vertices;
    vertices.reserve(kMaxDebugVertices);
    lineVertexCount_ = 0;
    if (showActorBounds) {
        const Vector<float, 3> green = {0.0f, 1.0f, 0.0f};
        const Vector<float, 3> red = {1.0f, 0.0f, 0.0f};
        std::vector<Vector<float, 3>> mins;
        std::vector<Vector<float, 3>> maxs;
        mins.reserve(renderer_.actors.size());
        maxs.reserve(renderer_.actors.size());
        for (size_t i = 0; i < renderer_.actors.size(); ++i) {
            RenderActor actor = renderer_.actors[i];
            if (actor.meshID >= allMeshMaxAbsoluteValues.size()) {
                mins.push_back({0, 0, 0});
                maxs.push_back({0, 0, 0});
                continue;
            }
            const MeshAxisMaxAbsoluteValues& bounds =
                allMeshMaxAbsoluteValues[actor.meshID];
            const Vector<float, 3> center = {
                bounds.origin_offset_x,
                bounds.origin_offset_y,
                bounds.origin_offset_z
            };
            const Vector<float, 3> half =
                {bounds.absolute_x, bounds.absolute_y, bounds.absolute_z};
            Vector<float, 3> localCorners[8] = {
                center + Vector<float, 3>(-half[0], -half[1], -half[2]),
                center + Vector<float, 3>(half[0], -half[1], -half[2]),
                center + Vector<float, 3>(half[0], half[1], -half[2]),
                center + Vector<float, 3>(-half[0], half[1], -half[2]),
                center + Vector<float, 3>(-half[0], -half[1], half[2]),
                center + Vector<float, 3>(half[0], -half[1], half[2]),
                center + Vector<float, 3>(half[0], half[1], half[2]),
                center + Vector<float, 3>(-half[0], half[1], half[2])
            };
            Vector<float, 3> mn =
                fromVec4(toVec4(localCorners[0], 1.0f) * actor.modelMatrix);
            Vector<float, 3> mx = mn;
            for (int c = 1; c < 8; ++c) {
                Vector<float, 3> w =
                    fromVec4(toVec4(localCorners[c], 1.0f) * actor.modelMatrix);
                for (int a = 0; a < 3; ++a) {
                    mn[a] = std::min(mn[a], w[a]);
                    mx[a] = std::max(mx[a], w[a]);
                }
            }
            mins.push_back(mn);
            maxs.push_back(mx);
        }
        std::vector<bool> collides(renderer_.actors.size(), false);
        for (size_t i = 0; i < renderer_.actors.size(); ++i) {
            for (size_t j = i + 1; j < renderer_.actors.size(); ++j) {
                // Non-strict: the collision system resolves contact by pushing
                // the mover back to exactly touch the target, so overlapping
                // boxes (<) alone misses face-to-face contact.
                if (mins[i][0] <= maxs[j][0] && maxs[i][0] >= mins[j][0]
                    && mins[i][1] <= maxs[j][1] && maxs[i][1] >= mins[j][1]
                    && mins[i][2] <= maxs[j][2] && maxs[i][2] >= mins[j][2]) {
                    collides[i] = true;
                    collides[j] = true;
                }
            }
        }
        for (size_t i = 0; i < renderer_.actors.size(); ++i) {
            if (renderer_.actors[i].meshID >= allMeshMaxAbsoluteValues.size()) {
                continue;
            }
            const MeshAxisMaxAbsoluteValues& bounds =
                allMeshMaxAbsoluteValues[renderer_.actors[i].meshID];
            const Vector<float, 3> center = {
                bounds.origin_offset_x,
                bounds.origin_offset_y,
                bounds.origin_offset_z
            };
            const Vector<float, 3> half =
                {bounds.absolute_x, bounds.absolute_y, bounds.absolute_z};
            Vector<float, 3> localCorners[8] = {
                center + Vector<float, 3>(-half[0], -half[1], -half[2]),
                center + Vector<float, 3>(half[0], -half[1], -half[2]),
                center + Vector<float, 3>(half[0], half[1], -half[2]),
                center + Vector<float, 3>(-half[0], half[1], -half[2]),
                center + Vector<float, 3>(-half[0], -half[1], half[2]),
                center + Vector<float, 3>(half[0], -half[1], half[2]),
                center + Vector<float, 3>(half[0], half[1], half[2]),
                center + Vector<float, 3>(-half[0], half[1], half[2])
            };
            Vector<float, 3> worldCorners[8];
            for (int c = 0; c < 8; ++c) {
                worldCorners[c] = fromVec4(
                    toVec4(localCorners[c], 1.0f)
                    * renderer_.actors[i].modelMatrix
                );
            }
            pushBox(vertices, worldCorners, collides[i] ? red : green);
        }
    }

    if (showLightFrustums) {
        const Vector<float, 3> yellow = {1.0f, 1.0f, 0.0f};
        for (uint32_t i = 0; i < renderer_.directionalLightNumber; ++i) {
            Vector<float, 3> corners[8];
            Matrix<float, 4> inverseLight =
                inverse_matrix_4x4(renderer_.dirLightSpaceMatrix[i]);
            for (int c = 0; c < 8; ++c) {
                const float s = (c & 4) ? 1.0f : -1.0f; // z (near/far).
                const float u = (c & 2) ? 1.0f : -1.0f; // y.
                const float v = (c & 1) ? 1.0f : -1.0f; // x.
                corners[c] =
                    fromVec4(Vector<float, 4>(u, v, s, 1.0f) * inverseLight);
            }
            pushBox(vertices, corners, yellow);
        }
        const Vector<float, 3> cyan = {0.0f, 1.0f, 1.0f};
        for (uint32_t i = 0; i < renderer_.spotLightNumber; ++i) {
            Vector<float, 3> corners[8];
            Matrix<float, 4> inverseLight =
                inverse_matrix_4x4(renderer_.spotLightSpaceMatrix[i]);
            for (int c = 0; c < 8; ++c) {
                const float s = (c & 4) ? 1.0f : -1.0f;
                const float u = (c & 2) ? 1.0f : -1.0f;
                const float v = (c & 1) ? 1.0f : -1.0f;
                corners[c] =
                    fromVec4(Vector<float, 4>(u, v, s, 1.0f) * inverseLight);
            }
            pushBox(vertices, corners, cyan);
        }
    }

    if (showSpatialGrid) {
        const auto& grid = glvm::world.spatialGrid;
        const float halfChunk = grid.grid[0][0][0].size * 0.5f;
        const float cross = 1.5f;
        for (uint32_t z = 0; z < grid.depth; ++z) {
            for (uint32_t y = 0; y < grid.height; ++y) {
                for (uint32_t x = 0; x < grid.width; ++x) {
                    const auto& chunk = grid.grid[z][y][x];
                    const size_t count = chunk.entities.size();
                    if (count == 0) {
                        continue;
                    }
                    // Only occupied cells, colored by entity count.
                    const Vector<float, 3> color = count == 1
                        ? Vector<float, 3>(0.0f, 1.0f, 0.0f)
                        : count <= 3 ? Vector<float, 3>(1.0f, 1.0f, 0.0f)
                                     : Vector<float, 3>(1.0f, 0.0f, 0.0f);
                    const Vector<float, 3> center = chunk.position
                        + Vector<float, 3>(halfChunk, halfChunk, halfChunk);
                    pushLine(
                        vertices,
                        center - Vector<float, 3>(cross, 0, 0),
                        center + Vector<float, 3>(cross, 0, 0),
                        color
                    );
                    pushLine(
                        vertices,
                        center - Vector<float, 3>(0, cross, 0),
                        center + Vector<float, 3>(0, cross, 0),
                        color
                    );
                    pushLine(
                        vertices,
                        center - Vector<float, 3>(0, 0, cross),
                        center + Vector<float, 3>(0, 0, cross),
                        color
                    );
                }
            }
        }
    }

    lineVertexCount_ = static_cast<uint32_t>(vertices.size());

    if (!vertices.empty() && vertexBufferMapped_) {
        const size_t bytes = vertices.size() * sizeof(DebugVertex);
        const size_t capacity = kMaxDebugVertices * sizeof(DebugVertex);
        memcpy(vertexBufferMapped_, vertices.data(), std::min(bytes, capacity));
    }
}

void ImGuiOverlay::createRenderPass() {
    VkAttachmentDescription colorAttachment {};
    colorAttachment.format = renderer_.swapChainImageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    VkAttachmentReference colorReference {};
    colorReference.attachment = 0;
    colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    VkSubpassDescription subpass {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorReference;
    VkSubpassDependency dependencies[2] {};
    dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[0].dstSubpass = 0;
    dependencies[0].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
        | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT
        | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    dependencies[0].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    dependencies[0].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
        | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    dependencies[0].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    dependencies[1].srcSubpass = 0;
    dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[1].srcStageMask =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[1].dstAccessMask = 0;
    VkRenderPassCreateInfo renderPassInfo {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 2;
    renderPassInfo.pDependencies = dependencies;
    if (vkCreateRenderPass(
            renderer_.device,
            &renderPassInfo,
            nullptr,
            &renderPass_
        )
        != VK_SUCCESS) {
        throw std::runtime_error("failed to create imgui overlay render pass!");
    }
}

void ImGuiOverlay::createLinePipeline() {
    auto createShaderModule = [&](const char* path) {
        std::ifstream file(path, std::ios::ate | std::ios::binary);
        if (!file.is_open()) {
            throw std::runtime_error(
                std::string("failed to open shader: ") + path
            );
        }
        file.seekg(0, std::ios::beg);
        std::vector<char> code(
            (std::istreambuf_iterator<char>(file)),
            std::istreambuf_iterator<char>()
        );
        VkShaderModuleCreateInfo createInfo {};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
        VkShaderModule module;
        if (vkCreateShaderModule(renderer_.device, &createInfo, nullptr, &module)
            != VK_SUCCESS) {
            throw std::runtime_error("failed to create shader module!");
        }
        return module;
    };

    VkShaderModule vert = createShaderModule(
        "../../../crates/glvm2/assets/shaders/debug/debug_vert.spv"
    );
    VkShaderModule frag = createShaderModule(
        "../../../crates/glvm2/assets/shaders/debug/debug_frag.spv"
    );

    VkPipelineShaderStageCreateInfo shaderStages[2] {};
    shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    shaderStages[0].module = vert;
    shaderStages[0].pName = "main";
    shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    shaderStages[1].module = frag;
    shaderStages[1].pName = "main";

    VkVertexInputBindingDescription bindingDescription {};
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(DebugVertex);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions {};
    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[0].offset = offsetof(DebugVertex, x);
    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[1].offset = offsetof(DebugVertex, r);

    VkPipelineVertexInputStateCreateInfo vertexInputInfo {};
    vertexInputInfo.sType =
        VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.vertexAttributeDescriptionCount =
        static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

    VkPipelineInputAssemblyStateCreateInfo inputAssembly {};
    inputAssembly.sType =
        VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkPipelineViewportStateCreateInfo viewportState {};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    VkPipelineRasterizationStateCreateInfo rasterizer {};
    rasterizer.sType =
        VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_NONE;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multisampling {};
    multisampling.sType =
        VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineColorBlendAttachmentState colorBlendAttachment {};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT
        | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT
        | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_TRUE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.dstColorBlendFactor =
        VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor =
        VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo colorBlending {};
    colorBlending.sType =
        VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;

    VkPushConstantRange pushConstantRange {};
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(Matrix<float, 4>);

    VkPipelineLayoutCreateInfo layoutInfo {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    layoutInfo.setLayoutCount = 0;
    layoutInfo.pSetLayouts = nullptr;
    layoutInfo.pushConstantRangeCount = 1;
    layoutInfo.pPushConstantRanges = &pushConstantRange;

    if (vkCreatePipelineLayout(
            renderer_.device,
            &layoutInfo,
            nullptr,
            &lineLayout_
        )
        != VK_SUCCESS) {
        throw std::runtime_error("failed to create debug line pipeline layout!");
    }

    VkPipelineDepthStencilStateCreateInfo depthStencil {};
    depthStencil.sType =
        VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_FALSE;
    depthStencil.depthWriteEnable = VK_FALSE;

    VkDynamicState dynamicStates[] = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };
    VkPipelineDynamicStateCreateInfo dynamicState {};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = 2;
    dynamicState.pDynamicStates = dynamicStates;

    VkGraphicsPipelineCreateInfo pipelineInfo {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = lineLayout_;
    pipelineInfo.renderPass = renderPass_;
    pipelineInfo.subpass = 0;

    if (vkCreateGraphicsPipelines(
            renderer_.device,
            VK_NULL_HANDLE,
            1,
            &pipelineInfo,
            nullptr,
            &linePipeline_
        )
        != VK_SUCCESS) {
        throw std::runtime_error("failed to create debug line pipeline!");
    }

    vkDestroyShaderModule(renderer_.device, vert, nullptr);
    vkDestroyShaderModule(renderer_.device, frag, nullptr);
}

void ImGuiOverlay::createVertexBuffer() {
    const VkDeviceSize bufferSize = kMaxDebugVertices * sizeof(DebugVertex);
    renderer_.createBuffer(
        bufferSize,
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
            | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        vertexBuffer_,
        vertexBufferMemory_
    );
    vkMapMemory(
        renderer_.device,
        vertexBufferMemory_,
        0,
        bufferSize,
        0,
        &vertexBufferMapped_
    );
}

} // namespace glvm

#ifdef VK_USE_PLATFORM_WAYLAND_KHR
#endif

namespace glvm {
CVulkanRenderer::CVulkanRenderer() {
    imguiOverlay = new ImGuiOverlay(*this);
}

CVulkanRenderer::~CVulkanRenderer() {
    cleanup();
    delete imguiOverlay;
    imguiOverlay = nullptr;
}

void CVulkanRenderer::draw() {
    imguiOverlay->newFrame();
    mainRenderDrawFrame();
}

void CVulkanRenderer::SetViewMatrix(Matrix<float, 4> _viewMatrix) {
    viewMatrix = _viewMatrix;
}

void CVulkanRenderer::SetProjectionMatrix(Matrix<float, 4> _projectionMatrix) {
    projectionMatrix = _projectionMatrix;
}

void CVulkanRenderer::createTextureImage() {
    uint32_t texWidth, texHeight;
    [[maybe_unused]] uint32_t texChannels;

    unsigned int readableTextureDescriptorBindingIndex =
        descriptorSetsConfig[DescriptorSetDataLink::RIDABLE_TEXTURES]
            .descriptorsBindingsIDs[0];
    for (unsigned int i = 0; i < initializeTextureData_.size(); ++i) {
        VkDeviceSize imageSize {};
        unsigned char* pixels;
        [[maybe_unused]] const char* path_to_stb_image = nullptr;

#ifndef STB_IMAGE_IMPLEMENTATION
        imageSize = initializeTextureData_[i].dat_length_;
        pixels = initializeTextureData_[i].u_iData_;
        texWidth = initializeTextureData_[i].iWidth_;
        texHeight = initializeTextureData_[i].iHeight_;
#endif

#ifdef STB_IMAGE_IMPLEMENTATION
        path_to_stb_image = initializeTextureData_[i].path_to_image;
        pixels = stbi_load(
            path_to_stb_image,
            reinterpret_cast<int*>(&texWidth),
            reinterpret_cast<int*>(&texHeight),
            reinterpret_cast<int*>(&texChannels),
            STBI_rgb_alpha
        );
        imageSize = texWidth * texHeight * 4;
#endif

        if (!pixels) {
            throw std::runtime_error("failed to load texture image!");
        }

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        createBuffer(
            imageSize,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
                | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            stagingBuffer,
            stagingBufferMemory
        );

        void* data;
        vkMapMemory(device, stagingBufferMemory, 0, imageSize, 0, &data);
        memcpy(data, pixels, static_cast<size_t>(imageSize));
        vkUnmapMemory(device, stagingBufferMemory);
        VK_Image textureImage = {
            .image = VkImage {},
            .deviceMemory = VkDeviceMemory {},
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .createFlags = 0,
            .memoryPropertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            .usageFlags =
                VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            .aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT,
            .format = VK_FORMAT_R8G8B8A8_SRGB,
            .tiling = VK_IMAGE_TILING_OPTIMAL,
            .arrayLayers = 1,
            .width = texWidth,
            .height = texHeight
        };

        createImage(textureImage);

        transitionImageLayout(
            textureImage.image,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
        );
        copyBufferToImage(
            stagingBuffer,
            textureImage.image,
            static_cast<uint32_t>(texWidth),
            static_cast<uint32_t>(texHeight)
        );
        transitionImageLayout(
            textureImage.image,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        );
        *GPUDescriptors
             [descriptorBindingsConfig[readableTextureDescriptorBindingIndex]
                  .globalDescriptorOffset
              + i]
                 .GPUImage = textureImage;

        vkDestroyBuffer(device, stagingBuffer, nullptr);
        vkFreeMemory(device, stagingBufferMemory, nullptr);
    }
}

void CVulkanRenderer::recreateSwapChain() {
    vkDeviceWaitIdle(device);

#ifdef VK_USE_PLATFORM_XCB_KHR
    Window->configureWindow();
#endif

    imguiOverlay->destroySwapChainResources();
    cleanupSwapChain();

    createSwapChain();
    Window->width = swapChainExtent.width;
    Window->height = swapChainExtent.height;
    aspectRate = (float)Window->width / (float)Window->height;
    createImageViews();
    createDepthResources();
    createDirectionalLightShadowMapDepthResources();
    createSpotLightShadowMapDepthResources();
    createPointLightShadowMapDepthResources();
    createFramebuffers();
    createMainRenderDescriptorSets();
    imguiOverlay->createSwapChainResources();
}

void CVulkanRenderer::SetMeshData(
    std::vector<const char*> _pathsArray,
    std::vector<const char*> pathsGLTF
) {
    for (unsigned int i = 0; i < _pathsArray.size(); ++i) {
        pathsArray_.push_back(_pathsArray[i]);
    }

    for (unsigned int i = 0; i < pathsGLTF.size(); ++i) {
        pathsGLTF_.push_back(pathsGLTF[i]);
    }
}

void CVulkanRenderer::run() {
    VkConfigInitializer();
    descriptorSetBuilder();
    pipelineBuilder();
    renderPassesBuilder();
    renderThreadPool = new ThreadPool(3);
    startTime = std::chrono::steady_clock::now();

    initWindow();
    initVulkan();
}

void CVulkanRenderer::initWindow() {
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
    Window = initializeWaylandWindow();

    createWaylandSurfaceInfo.display = Window->display;
    createWaylandSurfaceInfo.surface = Window->wl_surface;
    aspectRate = (float)Window->width / (float)Window->height;

    if (createWaylandSurfaceInfo.display == NULL) {
        std::cout << "DISPLAY NULL" << std::endl;
    } else if (createWaylandSurfaceInfo.surface == NULL) {
        std::cout << "SURFACE NULL" << std::endl;
    }

    createWaylandSurfaceInfo.sType =
        VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR;
    createWaylandSurfaceInfo.pNext = nullptr;
    createWaylandSurfaceInfo.flags = 0;
#endif

#ifdef VK_USE_PLATFORM_XLIB_KHR
    Window = new glvm::WindowXVulkan();
    createXlibSurfaceInfo.dpy = Window->GetDisplay();
    createXlibSurfaceInfo.window = Window->GetWindow();
    aspectRate = (float)Window->width / (float)Window->height;

    createXlibSurfaceInfo.sType =
        VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
    createXlibSurfaceInfo.pNext = nullptr;
    createXlibSurfaceInfo.flags = 0;
#endif

#ifdef VK_USE_PLATFORM_XCB_KHR
    Window = new glvm::WindowXCBVulkan();
    createXcbSurfaceInfo.window = Window->GetWindow();
    createXcbSurfaceInfo.connection = Window->GetConnection();
    aspectRate = (float)Window->width / (float)Window->height;

    createXcbSurfaceInfo.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
    createXcbSurfaceInfo.pNext = nullptr;
    createXcbSurfaceInfo.flags = 0;
#endif

#ifdef VK_USE_PLATFORM_WIN32_KHR
    Window = new glvm::WindowWinVulkan();
    createWin32SurfaceInfo.hwnd = Window->GetModernWindowHWND();
    aspectRate = (float)Window->width / (float)Window->height;

    createWin32SurfaceInfo.sType =
        VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    createWin32SurfaceInfo.pNext = nullptr;
    createWin32SurfaceInfo.flags = 0;
#endif
}

void CVulkanRenderer::initializeGameLevelVertices() {
    for (unsigned int m = 0; m < levelGeneratedVertices.size(); ++m) {
        aVertices_.push_back(levelGeneratedVertices[m]);
        aIndices_.push_back(levelGeneratedIndices[m]);
        jointMatricesPerMesh.push_back({});
        frames.push_back({});
        for (int i = 0; i < 64; ++i) {
            frames[frames.size() - 1].push_back(0.0f);
        }
        int maximumJoints = 64;
        std::vector<std::vector<Matrix<float, 4>>> jointMatrices;
        for (int i = 0; i < maximumJoints; ++i) {
            std::vector<Matrix<float, 4>> globalAllFrameNodeMatrix;
            int numberOfFrames = 64;
            for (int j = 0; j < numberOfFrames; ++j) {
                Matrix<float, 4> unitMatrix(1.0f);
                globalAllFrameNodeMatrix.push_back(unitMatrix);
            }

            jointMatrices.push_back(globalAllFrameNodeMatrix);
        }
        jointMatricesPerMesh[jointMatricesPerMesh.size() - 1] = jointMatrices;

        uint32_t nextIndexGLTF = wavefrontObjCounter + gltfCounter + m;

        vertexBufferContainer.emplace_back();
        vertexBufferMemoryContainer.emplace_back();
        createVertexBuffer(
            vertexBufferContainer[nextIndexGLTF],
            vertexBufferMemoryContainer[nextIndexGLTF],
            aVertices_[nextIndexGLTF]
        );

        indexBufferContainer.emplace_back();
        indexBufferMemoryContaner.emplace_back();
        createIndexBuffer(
            indexBufferContainer[nextIndexGLTF],
            indexBufferMemoryContaner[nextIndexGLTF],
            aIndices_[nextIndexGLTF]
        );
    }
}

void CVulkanRenderer::initVulkan() {
    createInstance();
    setupDebugMessenger();
    createSurface();
    pickPhysicalDevice();
    createLogicalDevice();
    createSwapChain();
    createImageViews();
    createMainRenderPass();
    createDescriptorSetLayout();
    createGraphicsPipeline();
    createCommandPool(mainRenderCommandPool);
    const uint32_t secondaryBuffersCommandPoolsNumber = 3;
    secondaryBuffersCommandPools.resize(secondaryBuffersCommandPoolsNumber);
    for (uint32_t i = 0; i < secondaryBuffersCommandPools.size(); ++i) {
        createCommandPool(secondaryBuffersCommandPools[i]);
    }
    createDepthResources();
    createDirectionalLightShadowMapDepthResources();
    createSpotLightShadowMapDepthResources();
    createPointLightShadowMapDepthResources();
    createFramebuffers();
    createTextureImage();
    createTextureImageView();
    createTextureSampler();
    createShadowMapSampler();
    initializeVertexBuffersWithWavefrontData();
    initializeVertexBuffersWithGLTFData();
    initializeVertexBuffersWithFontData();

    createMainRenderUniformBuffers();
    createMainRenderDescriptorPool();
    createMainRenderDescriptorSets();
    imguiOverlay->init();
    setDebugObjectNames(
        device,
        vertexBufferContainer,
        indexBufferContainer,
        GPUDescriptors,
        fontIndicesContainer,
        fontVertexBufferContainer,
        fontIndexBufferContainer
    );
    const uint32_t mainRenderCommandBuffersNumber = 1;
    createCommandBuffers(
        mainRenderCommandPool,
        mainRenderCommandBuffers,
        mainRenderCommandBuffersNumber,
        VK_COMMAND_BUFFER_LEVEL_PRIMARY
    );

    createCommandBuffers(
        secondaryBuffersCommandPools[0],
        directionalLightSecondaryCommandBuffers,
        directionalLightNumber,
        VK_COMMAND_BUFFER_LEVEL_SECONDARY
    );
    createCommandBuffers(
        secondaryBuffersCommandPools[1],
        spotLightSecondaryCommandBuffers,
        spotLightNumber,
        VK_COMMAND_BUFFER_LEVEL_SECONDARY
    );
    createCommandBuffers(
        secondaryBuffersCommandPools[2],
        pointLightSecondaryCommandBuffers,
        pointLightNumber * 6 * 16,
        VK_COMMAND_BUFFER_LEVEL_SECONDARY
    );
    createSyncObjects(
        imageAvailableSemaphores,
        renderFinishedSemaphores,
        inFlightFences
    );
}

void CVulkanRenderer::initializeVertexBuffersWithWavefrontData() {
    for (unsigned int m = 0; m < pathsArray_.size(); ++m) {
        vertexBufferContainer.emplace_back();
        vertexBufferMemoryContainer.emplace_back();
        createVertexBuffer(
            vertexBufferContainer[m],
            vertexBufferMemoryContainer[m],
            aVertices_[m]
        );

        indexBufferContainer.emplace_back();
        indexBufferMemoryContaner.emplace_back();
        createIndexBuffer(
            indexBufferContainer[m],
            indexBufferMemoryContaner[m],
            aIndices_[m]
        );
        ++wavefrontObjCounter;
    }
}

void CVulkanRenderer::initializeVertexBuffersWithGLTFData() {
    for (unsigned int m = 0; m < pathsGLTF_.size(); ++m) {
        uint32_t nextIndexGLTF = wavefrontObjCounter + m;
        vertexBufferContainer.emplace_back();
        vertexBufferMemoryContainer.emplace_back();
        createVertexBuffer(
            vertexBufferContainer[nextIndexGLTF],
            vertexBufferMemoryContainer[nextIndexGLTF],
            aVertices_[nextIndexGLTF]
        );

        indexBufferContainer.emplace_back();
        indexBufferMemoryContaner.emplace_back();
        createIndexBuffer(
            indexBufferContainer[nextIndexGLTF],
            indexBufferMemoryContaner[nextIndexGLTF],
            aIndices_[nextIndexGLTF]
        );
        ++gltfCounter;
    }
}

void CVulkanRenderer::initializeVertexBuffersWithFontData() {
    for (unsigned int i = 0; i < symbolGVerticesContainer.size(); ++i) {
        const unsigned int nextBufferIndex = fontIndicesContainer[i];
        std::vector<Vertex> symbol_g_vertices = symbolGVerticesContainer[i];

        createVertexBuffer(
            fontVertexBufferContainer[nextBufferIndex],
            fontVertexBufferMemoryContainer[nextBufferIndex],
            symbol_g_vertices
        );
        createIndexBuffer(
            fontIndexBufferContainer[nextBufferIndex],
            fontIndexBufferMemoryContaner[nextBufferIndex],
            symbol_g_indices
        );
    }
}

void CVulkanRenderer::clearVK_Image(VK_Image* textureImages) {
    vkDestroySampler(device, textureImages->sampler, nullptr);
    for (unsigned int j = 0; j < textureImages->views.size(); ++j) {
        vkDestroyImageView(device, textureImages->views[j], nullptr);
    }

    textureImages->views.clear();

    vkDestroyImage(device, textureImages->image, nullptr);
    vkFreeMemory(device, textureImages->deviceMemory, nullptr);
}

void CVulkanRenderer::cleanupSwapChain() {
    vkDeviceWaitIdle(device);
    vkDestroyImageView(device, mainDepthImageView, nullptr);
    vkDestroyImage(device, mainDepthPipelineImage, nullptr);
    vkFreeMemory(device, mainDepthPipelineImageMemory, nullptr);

    for (VkFramebuffer& framebuffer : swapChainFramebuffers) {
        vkDestroyFramebuffer(device, framebuffer, nullptr);
    }

    for (VkFramebuffer& framebuffer : directionalLightShadowMapFrameBuffers) {
        vkDestroyFramebuffer(device, framebuffer, nullptr);
    }

    for (VkFramebuffer& framebuffer : spotLightShadowMapFrameBuffers) {
        vkDestroyFramebuffer(device, framebuffer, nullptr);
    }

    for (std::vector<VkFramebuffer>& inner_vector :
         pointLightShadowMapFrameBuffers) {
        for (VkFramebuffer& framebuffer : inner_vector) {
            vkDestroyFramebuffer(device, framebuffer, nullptr);
        }
    }

    for (VkImageView& imageView : swapChainImageViews) {
        vkDestroyImageView(device, imageView, nullptr);
    }

    vkDestroySwapchainKHR(device, swapChain, nullptr);
}

void CVulkanRenderer::cleanup() {
    cleanupSwapChain();
    imguiOverlay->shutdown();

    for (unsigned int i = 0, j = 0; i < GPUDescriptors.size(); ++j) {
        if (descriptorBindingsConfig[j].vkType
            == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) {
            vkDestroyBuffer(
                device,
                GPUDescriptors[i].GPUBuffer->buffer,
                nullptr
            );
            vkFreeMemory(
                device,
                GPUDescriptors[i].GPUBuffer->deviceMemory,
                nullptr
            );
            delete GPUDescriptors[i].GPUBuffer;
            GPUDescriptors[i].GPUBuffer = nullptr;
        } else if (
            descriptorBindingsConfig[j].vkType
            == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
        ) {
            for (unsigned int n = i;
                 n < i + descriptorBindingsConfig[j].shaderDescriptorsNumber;
                 ++n) {
                if (GPUDescriptors[n].GPUImage->views.size()) {
                    clearVK_Image(GPUDescriptors[n].GPUImage);
                }

                delete GPUDescriptors[n].GPUImage;
                GPUDescriptors[n].GPUImage = nullptr;
            }
        }
        i = i + descriptorBindingsConfig[j].shaderDescriptorsNumber;
    }

    vkDestroyBuffer(device, hudUniformBuffer, nullptr);
    vkFreeMemory(device, hudUniformBuffersMemory, nullptr);
    vkDestroyBuffer(device, fontUniformBuffer, nullptr);
    vkFreeMemory(device, fontUniformBuffersMemory, nullptr);
    vkDestroyBuffer(device, hudScreenUniformBuffer, nullptr);
    vkFreeMemory(device, hudScreenUniformBuffersMemory, nullptr);
    vkDestroyBuffer(device, uiUniformBuffer, nullptr);
    vkFreeMemory(device, uiUniformBuffersMemory, nullptr);
    vkDestroyBuffer(device, uiIconsUniformBuffer, nullptr);
    vkFreeMemory(device, uiIconsUniformBuffersMemory, nullptr);
    vkDestroyBuffer(
        device,
        shadowMapDirectionalLightModelMatrixUniformBuffer,
        nullptr
    );
    vkFreeMemory(
        device,
        shadowMapDirectionalLightModelMatrixUniformBuffersMemory,
        nullptr
    );
    vkDestroyBuffer(
        device,
        shadowMapPointLightModelMatrixUniformBuffer,
        nullptr
    );
    vkFreeMemory(
        device,
        shadowMapPointLightModelMatrixUniformBuffersMemory,
        nullptr
    );
    vkDestroyBuffer(device, shadowMapSpotLightModelMatrixUniformBuffer, nullptr);
    vkFreeMemory(
        device,
        shadowMapSpotLightModelMatrixUniformBuffersMemory,
        nullptr
    );
    vkDestroyBuffer(device, virtualTexturesUniformBuffer, nullptr);
    vkFreeMemory(device, virtualTexturesUniformBufferMemory, nullptr);

    for (size_t j = 0; j < vertexBufferContainer.size(); ++j) {
        vkDestroyBuffer(device, vertexBufferContainer[j], nullptr);
        vkFreeMemory(device, vertexBufferMemoryContainer[j], nullptr);
    }
    for (size_t j = 0; j < indexBufferContainer.size(); ++j) {
        vkDestroyBuffer(device, indexBufferContainer[j], nullptr);
        vkFreeMemory(device, indexBufferMemoryContaner[j], nullptr);
    }
    for (size_t j = 0; j < fontIndicesContainer.size(); ++j) {
        vkDestroyBuffer(
            device,
            fontVertexBufferContainer[fontIndicesContainer[j]],
            nullptr
        );
        vkFreeMemory(
            device,
            fontVertexBufferMemoryContainer[fontIndicesContainer[j]],
            nullptr
        );
    }
    for (size_t j = 0; j < fontIndicesContainer.size(); ++j) {
        vkDestroyBuffer(
            device,
            fontIndexBufferContainer[fontIndicesContainer[j]],
            nullptr
        );
        vkFreeMemory(
            device,
            fontIndexBufferMemoryContaner[fontIndicesContainer[j]],
            nullptr
        );
    }
    vkDestroyBuffer(device, modelMatrixUniformBuffer, nullptr);
    vkFreeMemory(device, modelMatrixUniformBuffersMemory, nullptr);
    vkDestroyBuffer(device, lightDataUniformBuffer, nullptr);
    vkFreeMemory(device, lightDataUniformBuffersMemory, nullptr);

    vkDeviceWaitIdle(device);

    for (int i = 0; i < SpecificPipeline::PIPELINES_NUMBER; ++i) {
        vkDestroyRenderPass(device, renderPasses[i], nullptr);
    }

    for (unsigned int i = 0;
         i < DescriptorSetDataLink::DESCRIPTOR_CHUNKS_NUMBER;
         ++i) {
        vkDestroyDescriptorSetLayout(
            device,
            descriptorSetsConfig[i].setLayout,
            nullptr
        );
    }
    for (unsigned int i = 0; i < SpecificPipeline::PIPELINES_NUMBER; ++i) {
        vkDestroyPipeline(device, pipelineConfigs[i].pipeline, nullptr);
        vkDestroyPipelineLayout(
            device,
            pipelineConfigs[i].pipelineLayout,
            nullptr
        );
    }

    vkDestroySampler(device, textureSampler, nullptr);
    vkDestroySampler(device, shadowMapSampler, nullptr);
    for (unsigned int i = 0; i < textureImages.size(); ++i) {
        vkDestroySampler(device, textureImages[i].sampler, nullptr);
        for (unsigned int j = 0; j < textureImages[i].views.size(); ++j) {
            vkDestroyImageView(device, textureImages[i].views[j], nullptr);
        }

        vkDestroyImage(device, textureImages[i].image, nullptr);
        vkFreeMemory(device, textureImages[i].deviceMemory, nullptr);
    }

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
        vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
        vkDestroyFence(device, inFlightFences[i], nullptr);
    }

    for (size_t i = 0; i < swapChainImages.size(); ++i) {
        vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
    }

    vkDestroyCommandPool(device, directionalLightCommandPool, nullptr);
    vkDestroyCommandPool(device, spotLightCommandPool, nullptr);
    vkDestroyCommandPool(device, pointLightCommandPool, nullptr);
    vkDestroyCommandPool(device, mainRenderCommandPool, nullptr);
    vkDestroyCommandPool(device, fontCommandPool, nullptr);
    vkDestroyCommandPool(device, hudCommandPool, nullptr);
    vkDestroyCommandPool(device, hudScreenCommandPool, nullptr);
    vkDestroyCommandPool(device, uiCommandPool, nullptr);
    vkDestroyCommandPool(device, uiIconsCommandPool, nullptr);
    vkDestroyCommandPool(device, virtualTexturesCommandPool, nullptr);
    for (uint32_t i = 0; i < secondaryBuffersCommandPools.size(); ++i) {
        vkDestroyCommandPool(device, secondaryBuffersCommandPools[i], nullptr);
    }
    vkDestroyDescriptorPool(device, descriptorPool, nullptr);

    vkDeviceWaitIdle(device);
    vkDestroyDevice(device, nullptr);

    if (enableValidationLayers) {
        DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
    }

    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyInstance(instance, nullptr);
    Window->Close();
}

void CVulkanRenderer::createInstance() {
    if (enableValidationLayers && !checkValidationLayerSupport()) {
        throw std::runtime_error(
            "validation layers requested, but not available!"
        );
    }

    VkApplicationInfo appInfo {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Hello Triangle";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "Grey Lane Vertex Machine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    std::vector<const char*> extensions = getRequiredExtensions();

    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo {};
    if (enableValidationLayers) {
        createInfo.enabledLayerCount =
            static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();

        populateDebugMessengerCreateInfo(debugCreateInfo);
        createInfo.pNext =
            (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
    } else {
        createInfo.enabledLayerCount = 0;

        createInfo.pNext = nullptr;
    }

    if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
        throw std::runtime_error("failed to create instance!");
    }
}

void CVulkanRenderer::populateDebugMessengerCreateInfo(
    VkDebugUtilsMessengerCreateInfoEXT& createInfo
) {
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
}

void CVulkanRenderer::setupDebugMessenger() {
    if (!enableValidationLayers) {
        return;
    }

    VkDebugUtilsMessengerCreateInfoEXT createInfo;
    populateDebugMessengerCreateInfo(createInfo);

    if (CreateDebugUtilsMessengerEXT(
            instance,
            &createInfo,
            nullptr,
            &debugMessenger
        )
        != VK_SUCCESS) {
        throw std::runtime_error("failed to set up debug messenger!");
    }
}

void CVulkanRenderer::createSurface() {
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
    if (vkCreateWaylandSurfaceKHR(
            instance,
            &createWaylandSurfaceInfo,
            nullptr,
            &surface
        )
        != VK_SUCCESS) {
        throw std::runtime_error("failed to create window surface!");
    }
#endif

#ifdef VK_USE_PLATFORM_XLIB_KHR
    if (vkCreateXlibSurfaceKHR(
            instance,
            &createXlibSurfaceInfo,
            nullptr,
            &surface
        )
        != VK_SUCCESS) {
        throw std::runtime_error("failed to create window surface!");
    }
#endif

#ifdef VK_USE_PLATFORM_XCB_KHR
    if (vkCreateXcbSurfaceKHR(instance, &createXcbSurfaceInfo, nullptr, &surface)
        != VK_SUCCESS) {
        throw std::runtime_error("failed to create window surface!");
    }
#endif

#ifdef VK_USE_PLATFORM_WIN32_KHR
    if (vkCreateWin32SurfaceKHR(
            instance,
            &createWin32SurfaceInfo,
            nullptr,
            &surface
        )
        != VK_SUCCESS) {
        throw std::runtime_error("failed to create window surface!");
    }
#endif
}

void CVulkanRenderer::pickPhysicalDevice() {
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

    if (deviceCount == 0) {
        throw std::runtime_error("failed to find GPUs with Vulkan support!");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    for (const VkPhysicalDevice& device : devices) {
        VkPhysicalDeviceProperties prop;
        vkGetPhysicalDeviceProperties(device, &prop);

        if (isDeviceSuitable(device)) {
            physicalDevice = device;
            break;
        }
    }

    if (physicalDevice == VK_NULL_HANDLE) {
        throw std::runtime_error("failed to find a suitable GPU!");
    }
}

void CVulkanRenderer::createLogicalDevice() {
    QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {
        indices.graphics_family.value(),
        indices.present_family.value()
    };

    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queueCreateInfo {};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures {};
    deviceFeatures.samplerAnisotropy = VK_TRUE;

    VkDeviceCreateInfo createInfo {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    createInfo.queueCreateInfoCount =
        static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();

    createInfo.pEnabledFeatures = &deviceFeatures;

    createInfo.enabledExtensionCount =
        static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();

    if (enableValidationLayers) {
        createInfo.enabledLayerCount =
            static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    } else {
        createInfo.enabledLayerCount = 0;
    }

    if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device)
        != VK_SUCCESS) {
        throw std::runtime_error("failed to create logical device!");
    }

    vkGetDeviceQueue(device, indices.graphics_family.value(), 0, &graphicsQueue);
    vkGetDeviceQueue(device, indices.present_family.value(), 0, &presentQueue);
}

void CVulkanRenderer::createSwapChain() {
    SwapChainSupportDetails swapChainSupport =
        querySwapChainSupport(physicalDevice);

    VkSurfaceFormatKHR surfaceFormat =
        chooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode =
        chooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0
        && imageCount > swapChainSupport.capabilities.maxImageCount) {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo {};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface;

    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
    uint32_t queueFamilyIndices[] = {
        indices.graphics_family.value(),
        indices.present_family.value()
    };

    if (indices.graphics_family != indices.present_family) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;

    if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain)
        != VK_SUCCESS) {
        throw std::runtime_error("failed to create swap chain!");
    }

    vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
    swapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(
        device,
        swapChain,
        &imageCount,
        swapChainImages.data()
    );

    swapChainImageFormat = surfaceFormat.format;
    swapChainExtent = extent;
    Window->width = swapChainExtent.width;
    Window->height = swapChainExtent.height;
}

void CVulkanRenderer::createImageViews() {
    swapChainImageViews.resize(swapChainImages.size());

    for (uint32_t i = 0; i < swapChainImages.size(); i++) {
        VK_Image swapChainImage = {
            .image = swapChainImages[i],
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT,
            .format = swapChainImageFormat,
            .red = VK_COMPONENT_SWIZZLE_IDENTITY,
            .green = VK_COMPONENT_SWIZZLE_IDENTITY,
            .blue = VK_COMPONENT_SWIZZLE_IDENTITY,
            .alpha = VK_COMPONENT_SWIZZLE_IDENTITY,
            .arrayLayers = 1,
            .width = swapChainExtent.width,
            .height = swapChainExtent.height
        };

        swapChainImageViews[i] = createImageView(swapChainImage, 0, 1);
    }
}

void CVulkanRenderer::createMainRenderPass() {
    for (unsigned int j = 0; j < SpecificPipeline::PIPELINES_NUMBER; ++j) {
        for (unsigned int i = 0;
             i < renderPassConfigs[j].actualAttachmentDescriptionNumber;
             ++i) {
            if (renderPassConfigs[j].attachmentDescriptions[i].finalLayout
                    == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
                || renderPassConfigs[j].attachmentDescriptions[i].finalLayout
                    == VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL) {
                renderPassConfigs[j].attachmentDescriptions[i].format =
                    findDepthFormat();
            } else {
                renderPassConfigs[j].attachmentDescriptions[i].format =
                    swapChainImageFormat;
            }
        }

        VkSubpassDescription subpass {};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        for (unsigned int i = 0;
             i < renderPassConfigs[j].actualAttachmentReferenceNumber;
             ++i) {
            if (renderPassConfigs[j].attachmentReferences[i].layout
                == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
                subpass.colorAttachmentCount = 1;
                subpass.pColorAttachments =
                    &renderPassConfigs[j].attachmentReferences[i];
            } else if (
                renderPassConfigs[j].attachmentReferences[i].layout
                == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
            ) {
                subpass.pDepthStencilAttachment =
                    &renderPassConfigs[j].attachmentReferences[i];
            }
        }

        VkRenderPassCreateInfo renderPassInfo {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = static_cast<uint32_t>(
            renderPassConfigs[j].actualAttachmentDescriptionNumber
        );
        renderPassInfo.pAttachments =
            renderPassConfigs[j].attachmentDescriptions;
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount =
            renderPassConfigs[j].actualSubpassDependencyNumber;
        renderPassInfo.pDependencies = renderPassConfigs[j].subpassDependencies;
        if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPasses[j])
            != VK_SUCCESS) {
            throw std::runtime_error("failed to create render pass!");
        }
    }
}

void CVulkanRenderer::createDescriptorSetLayout() {
    for (int descriptorSetCounter = 0;
         descriptorSetCounter < DescriptorSetDataLink::DESCRIPTOR_CHUNKS_NUMBER;
         ++descriptorSetCounter) {
        DescriptorSet& descriptorSet =
            descriptorSetsConfig[descriptorSetCounter];
        std::vector<VkDescriptorSetLayoutBinding> bindings;
        for (uint32_t j = 0;
             j < descriptorSet.actualLinkedDescriptorBindingsNumber;
             ++j) {
            uint32_t currentDescriptorBindingID =
                descriptorSet.descriptorsBindingsIDs[j];
            VkDescriptorSetLayoutBinding modelMatrixUboLayout {};
            modelMatrixUboLayout.binding =
                descriptorBindingsConfig[currentDescriptorBindingID].binding;
            modelMatrixUboLayout.descriptorCount =
                descriptorBindingsConfig[currentDescriptorBindingID]
                    .shaderDescriptorsNumber;
            modelMatrixUboLayout.descriptorType =
                descriptorBindingsConfig[currentDescriptorBindingID].vkType;
            modelMatrixUboLayout.pImmutableSamplers = nullptr;
            modelMatrixUboLayout.stageFlags =
                descriptorBindingsConfig[currentDescriptorBindingID]
                    .shaderStageFlag;

            bindings.push_back(modelMatrixUboLayout);
        }

        VkDescriptorSetLayoutCreateInfo layoutInfo {};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.flags = 0;
        layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
        layoutInfo.pBindings = bindings.data();
        if (vkCreateDescriptorSetLayout(
                device,
                &layoutInfo,
                nullptr,
                &descriptorSet.setLayout
            )
            != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor set layout!");
        }
    }
}

void CVulkanRenderer::createGraphicsPipeline() {
    for (int graphicsPipelineCounter = 0;
         graphicsPipelineCounter < SpecificPipeline::PIPELINES_NUMBER;
         ++graphicsPipelineCounter) {
        Pipeline& pipeline = pipelineConfigs[graphicsPipelineCounter];
        VkRenderPass renderPass = renderPasses[graphicsPipelineCounter];
        std::vector<VkPipelineShaderStageCreateInfo> shaderStages;

        VkShaderModule vertShaderModule;
        VkShaderModule fragShaderModule;
        if (pipeline.vertShader != nullptr) {
            std::vector<char> vertShaderCode = readFile(pipeline.vertShader);
            vertShaderModule = createShaderModule(vertShaderCode);

            VkPipelineShaderStageCreateInfo vertShaderStageInfo {};
            vertShaderStageInfo.sType =
                VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
            vertShaderStageInfo.module = vertShaderModule;
            vertShaderStageInfo.pName = "main";

            shaderStages.push_back(vertShaderStageInfo);
        }

        if (pipeline.fragShader != nullptr) {
            std::vector<char> fragShaderCode = readFile(pipeline.fragShader);
            fragShaderModule = createShaderModule(fragShaderCode);

            VkPipelineShaderStageCreateInfo fragShaderStageInfo {};
            fragShaderStageInfo.sType =
                VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
            fragShaderStageInfo.module = fragShaderModule;
            fragShaderStageInfo.pName = "main";

            shaderStages.push_back(fragShaderStageInfo);
        }

        VkPipelineVertexInputStateCreateInfo vertexInputInfo {};
        vertexInputInfo.sType =
            VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

        vertexInputInfo.vertexBindingDescriptionCount = 1;
        vertexInputInfo.vertexAttributeDescriptionCount =
            static_cast<uint32_t>(pipeline.attributeDescriptions.size());
        vertexInputInfo.pVertexBindingDescriptions =
            &pipeline.bindingDescription;
        vertexInputInfo.pVertexAttributeDescriptions =
            pipeline.attributeDescriptions.data();

        VkPipelineInputAssemblyStateCreateInfo inputAssembly {};
        inputAssembly.sType =
            VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        VkPipelineViewportStateCreateInfo viewportState {};
        viewportState.sType =
            VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.scissorCount = 1;

        VkPipelineRasterizationStateCreateInfo rasterizer {};
        rasterizer.sType =
            VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizer.lineWidth = 1.0f;
        bool isShadowMapPipeline = graphicsPipelineCounter
                == SpecificPipeline::DIRECTIONAL_LIGHT_PIPELINE
            || graphicsPipelineCounter == SpecificPipeline::SPOT_LIGHT_PIPELINE
            || graphicsPipelineCounter
                == SpecificPipeline::POINT_LIGHT_PIPELINE;
        // Meshes are CW-wound; main pass keeps outer faces (cull "front"),
        // shadow pass keeps far-side faces (cull "back") so objects do not
        // self-shadow.
        if (isShadowMapPipeline) {
            rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
        } else if (
            graphicsPipelineCounter == SpecificPipeline::MAIN_RENDER_PIPELINE
        ) {
            rasterizer.cullMode = VK_CULL_MODE_FRONT_BIT;
        } else {
            // 2D/UI pipelines (HUD, crosshair, fonts, SDF) draw screen-space
            // quads; culling them makes the sprites disappear.
            rasterizer.cullMode = VK_CULL_MODE_NONE;
        }
        rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        rasterizer.depthBiasEnable = VK_FALSE;

        VkPipelineMultisampleStateCreateInfo multisampling {};
        multisampling.sType =
            VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        VkPipelineDepthStencilStateCreateInfo depthStencil {};
        depthStencil.sType =
            VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencil.depthTestEnable = VK_TRUE;
        depthStencil.depthWriteEnable = VK_TRUE;
        depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
        depthStencil.depthBoundsTestEnable = VK_FALSE;
        depthStencil.stencilTestEnable = VK_FALSE;

        VkPipelineColorBlendAttachmentState colorBlendAttachment {};
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT
            | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT
            | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_FALSE;

        VkPipelineColorBlendStateCreateInfo colorBlending {};
        colorBlending.sType =
            VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.logicOp = VK_LOGIC_OP_COPY;
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &colorBlendAttachment;
        colorBlending.blendConstants[0] = 0.0f;
        colorBlending.blendConstants[1] = 0.0f;
        colorBlending.blendConstants[2] = 0.0f;
        colorBlending.blendConstants[3] = 0.0f;

        std::vector<VkDynamicState> dynamicStates = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR
        };
        VkPipelineDynamicStateCreateInfo dynamicState {};
        dynamicState.sType =
            VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.dynamicStateCount =
            static_cast<uint32_t>(dynamicStates.size());
        dynamicState.pDynamicStates = dynamicStates.data();

        // Need to access inside pipeline and take ID for specific descriptor
        // set, then with that ID we got descriptor set and take it's layout.
        unsigned int descriptorLayoutsNumber =
            pipeline.actualLinkedDescriptorSetsNumber;
        std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
        for (unsigned i = 0; i < descriptorLayoutsNumber; ++i) {
            descriptorSetLayouts.push_back(
                descriptorSetsConfig[pipeline.linkedDescriptorSetIDs[i]].setLayout
            );
        }

        VkPipelineLayoutCreateInfo pipelineLayoutInfo {};
        pipelineLayoutInfo.sType =
            VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = descriptorLayoutsNumber;
        pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();

        if (vkCreatePipelineLayout(
                device,
                &pipelineLayoutInfo,
                nullptr,
                &pipeline.pipelineLayout
            )
            != VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline layout!");
        }

        VkGraphicsPipelineCreateInfo pipelineInfo {};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = shaderStages.size();
        pipelineInfo.pStages = shaderStages.data();
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pDepthStencilState = &depthStencil;
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.pDynamicState = &dynamicState;
        pipelineInfo.layout = pipeline.pipelineLayout;
        pipelineInfo.renderPass = renderPass;
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

        if (vkCreateGraphicsPipelines(
                device,
                VK_NULL_HANDLE,
                1,
                &pipelineInfo,
                nullptr,
                &pipeline.pipeline
            )
            != VK_SUCCESS) {
            throw std::runtime_error("failed to create graphics pipeline!");
        }

        if (pipeline.vertShader != nullptr) {
            vkDestroyShaderModule(device, vertShaderModule, nullptr);
        }

        if (pipeline.fragShader != nullptr) {
            vkDestroyShaderModule(device, fragShaderModule, nullptr);
        }
    }
}

void CVulkanRenderer::createFramebuffers() {
    // Main renderer frame buffers initialization.
    swapChainFramebuffers.resize(swapChainImageViews.size());
    for (size_t i = 0; i < swapChainImageViews.size(); ++i) {
        std::vector<VkImageView> mainRenderAttachments;
        mainRenderAttachments.push_back(swapChainImageViews[i]);
        mainRenderAttachments.push_back(mainDepthImageView);

        createRenderPassFramebuffers(
            mainRenderAttachments,
            renderPasses[SpecificPipeline::MAIN_RENDER_PIPELINE],
            swapChainFramebuffers[i],
            swapChainExtent.width,
            swapChainExtent.height
        );
    }

    // Directional lights shadow map renderer frame buffers initialization.
    unsigned int directionalLightDescriptorBindingIndex =
        descriptorSetsConfig[DescriptorSetDataLink::MAIN_RENDER_LIGHT_DATA_UBO]
            .descriptorsBindingsIDs[1];
    directionalLightShadowMapFrameBuffers.resize(DIRECTIONAL_LIGHTS_NUMBER);
    for (size_t i = 0; i < DIRECTIONAL_LIGHTS_NUMBER; ++i) {
        std::vector<VkImageView> directionalLightsRenderAttachments;
        directionalLightsRenderAttachments.push_back(
            (*GPUDescriptors
                  [descriptorBindingsConfig[directionalLightDescriptorBindingIndex]
                       .globalDescriptorOffset
                   + i]
                      .GPUImage)
                .views[0]
        );
        createRenderPassFramebuffers(
            directionalLightsRenderAttachments,
            renderPasses[SpecificPipeline::DIRECTIONAL_LIGHT_PIPELINE],
            directionalLightShadowMapFrameBuffers[i],
            FLAT_SHADOW_MAP_SIZE,
            FLAT_SHADOW_MAP_SIZE
        );
    }

    // Spot lights shadow map renderer frame buffers initialization.
    unsigned int spotLightDescriptorBindingIndex =
        descriptorSetsConfig[DescriptorSetDataLink::MAIN_RENDER_LIGHT_DATA_UBO]
            .descriptorsBindingsIDs[3];
    spotLightShadowMapFrameBuffers.resize(SPOT_LIGHTS_NUMBER);
    for (size_t i = 0; i < SPOT_LIGHTS_NUMBER; ++i) {
        std::vector<VkImageView> spotLightsRenderAttachments;
        spotLightsRenderAttachments.push_back(
            (*GPUDescriptors
                  [descriptorBindingsConfig[spotLightDescriptorBindingIndex]
                       .globalDescriptorOffset
                   + i]
                      .GPUImage)
                .views[0]
        );

        createRenderPassFramebuffers(
            spotLightsRenderAttachments,
            renderPasses[SpecificPipeline::SPOT_LIGHT_PIPELINE],
            spotLightShadowMapFrameBuffers[i],
            FLAT_SHADOW_MAP_SIZE,
            FLAT_SHADOW_MAP_SIZE
        );
    }

    // Point lights shadow map renderer frame buffers initialization.
    unsigned int descriptorBindingIndex =
        descriptorSetsConfig[DescriptorSetDataLink::MAIN_RENDER_LIGHT_DATA_UBO]
            .descriptorsBindingsIDs[2];
    pointLightShadowMapFrameBuffers.resize(POINT_LIGHTS_NUMBER);
    for (size_t j = 0; j < POINT_LIGHTS_NUMBER; ++j) {
        for (size_t m = 0; m < 6; ++m) {
            std::vector<VkImageView> pointLightsRenderAttachments;
            pointLightsRenderAttachments.push_back(
                (*GPUDescriptors
                      [descriptorBindingsConfig[descriptorBindingIndex]
                           .globalDescriptorOffset
                       + j]
                          .GPUImage)
                    .views[m]
            );
            pointLightShadowMapFrameBuffers[j].push_back({});
            createRenderPassFramebuffers(
                pointLightsRenderAttachments,
                renderPasses[SpecificPipeline::POINT_LIGHT_PIPELINE],
                pointLightShadowMapFrameBuffers[j][m],
                SHADOW_MAP_SIZE,
                SHADOW_MAP_SIZE
            );
        }
    }
}

void CVulkanRenderer::createRenderPassFramebuffers(
    std::vector<VkImageView>& attachments,
    VkRenderPass& renderPass_,
    VkFramebuffer& swapChainFramebuffer,
    uint32_t width,
    uint32_t height
) {
    VkFramebufferCreateInfo framebufferInfo {};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = renderPass_;
    framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    framebufferInfo.pAttachments = attachments.data();
    framebufferInfo.width = width;
    framebufferInfo.height = height;
    framebufferInfo.layers = 1;

    if (vkCreateFramebuffer(
            device,
            &framebufferInfo,
            nullptr,
            &swapChainFramebuffer
        )
        != VK_SUCCESS) {
        throw std::runtime_error("failed to create framebuffer!");
    }
}

void CVulkanRenderer::createCommandPool(VkCommandPool& commandPools) {
    QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);

    VkCommandPoolCreateInfo poolInfo {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphics_family.value();

    if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPools)
        != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics command pool!");
    }
}

void CVulkanRenderer::createDepthResources() {
    VkFormat depthFormat = findDepthFormat();

    VK_Image depthImage = {
        .image = VkImage {},
        .deviceMemory = VkDeviceMemory {},
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .createFlags = 0,
        .memoryPropertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        .usageFlags = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT
            | VK_IMAGE_USAGE_SAMPLED_BIT,
        .aspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT,
        .format = depthFormat,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .arrayLayers = 1,
        .width = swapChainExtent.width,
        .height = swapChainExtent.height,
    };

    createImage(depthImage);
    mainDepthPipelineImage = depthImage.image;
    mainDepthPipelineImageMemory = depthImage.deviceMemory;
    mainDepthImageView = createImageView(depthImage, 0, 1);
}

void CVulkanRenderer::createDirectionalLightShadowMapDepthResources() {
    unsigned int directionalLightDescriptorBindingIndex =
        descriptorSetsConfig[DescriptorSetDataLink::MAIN_RENDER_LIGHT_DATA_UBO]
            .descriptorsBindingsIDs[1];
    for (unsigned int i = 0; i < DIRECTIONAL_LIGHTS_NUMBER; ++i) {
        VK_Image depthImage = {
            .image = VkImage {},
            .deviceMemory = VkDeviceMemory {},
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .createFlags = 0,
            .memoryPropertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            .usageFlags = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT
                | VK_IMAGE_USAGE_SAMPLED_BIT,
            .aspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT,
            .format = findDepthFormat(),
            .tiling = VK_IMAGE_TILING_OPTIMAL,
            .arrayLayers = 1,
            .width = FLAT_SHADOW_MAP_SIZE,
            .height = FLAT_SHADOW_MAP_SIZE,
        };

        createImage(depthImage);

        VkCommandBuffer commandBuffer =
            beginSingleTimeCommands(mainRenderCommandPool);

        VkImageMemoryBarrier barrier {};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        barrier.newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = depthImage.image;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;

        VkPipelineStageFlags sourceStage;
        VkPipelineStageFlags destinationStage;

        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = 0;

        sourceStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

        vkCmdPipelineBarrier(
            commandBuffer,
            sourceStage,
            destinationStage,
            0,
            0,
            nullptr,
            0,
            nullptr,
            1,
            &barrier
        );

        endSingleTimeCommands(mainRenderCommandPool, commandBuffer);

        depthImage.views.push_back(createImageView(depthImage, 0, 1));
        setImageDebugObjectName(device, depthImage, "directional light");
        *GPUDescriptors
             [descriptorBindingsConfig[directionalLightDescriptorBindingIndex]
                  .globalDescriptorOffset
              + i]
                 .GPUImage = depthImage;
    }
}

void CVulkanRenderer::createSpotLightShadowMapDepthResources() {
    unsigned int spotLightDescriptorBindingIndex =
        descriptorSetsConfig[DescriptorSetDataLink::MAIN_RENDER_LIGHT_DATA_UBO]
            .descriptorsBindingsIDs[3];
    for (unsigned int i = 0; i < SPOT_LIGHTS_NUMBER; ++i) {
        VK_Image depthImage = {
            .image = VkImage {},
            .deviceMemory = VkDeviceMemory {},
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .createFlags = 0,
            .memoryPropertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            .usageFlags = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT
                | VK_IMAGE_USAGE_SAMPLED_BIT,
            .aspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT,
            .format = findDepthFormat(),
            .tiling = VK_IMAGE_TILING_OPTIMAL,
            .arrayLayers = 1,
            .width = FLAT_SHADOW_MAP_SIZE,
            .height = FLAT_SHADOW_MAP_SIZE,
        };

        createImage(depthImage);

        VkCommandBuffer commandBuffer =
            beginSingleTimeCommands(mainRenderCommandPool);

        VkImageMemoryBarrier barrier {};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        barrier.newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = depthImage.image;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;

        VkPipelineStageFlags sourceStage;
        VkPipelineStageFlags destinationStage;

        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = 0;

        sourceStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

        vkCmdPipelineBarrier(
            commandBuffer,
            sourceStage,
            destinationStage,
            0,
            0,
            nullptr,
            0,
            nullptr,
            1,
            &barrier
        );

        endSingleTimeCommands(mainRenderCommandPool, commandBuffer);

        depthImage.views.push_back(createImageView(depthImage, 0, 1));
        setImageDebugObjectName(device, depthImage, "spot light");
        *GPUDescriptors
             [descriptorBindingsConfig[spotLightDescriptorBindingIndex]
                  .globalDescriptorOffset
              + i]
                 .GPUImage = depthImage;
    }
}

void CVulkanRenderer::createPointLightShadowMapDepthResources() {
    unsigned int descriptorBindingIndex =
        descriptorSetsConfig[DescriptorSetDataLink::MAIN_RENDER_LIGHT_DATA_UBO]
            .descriptorsBindingsIDs[2];
    for (unsigned int i = 0; i < POINT_LIGHTS_NUMBER; ++i) {
        VK_Image depthImage = {
            .image = VkImage {},
            .deviceMemory = VkDeviceMemory {},
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .createFlags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT,
            .memoryPropertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            .usageFlags = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT
                | VK_IMAGE_USAGE_SAMPLED_BIT,
            .aspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT,
            .format = findDepthFormat(),
            .tiling = VK_IMAGE_TILING_OPTIMAL,
            .arrayLayers = 6,
            .width = SHADOW_MAP_SIZE,
            .height = SHADOW_MAP_SIZE
        };

        createImage(depthImage);

        for (unsigned int j = 0; j < 6; ++j) {
            VkCommandBuffer commandBuffer =
                beginSingleTimeCommands(mainRenderCommandPool);

            VkImageMemoryBarrier barrier {};
            barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            barrier.newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
            barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.image = depthImage.image;
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
            barrier.subresourceRange.baseMipLevel = 0;
            barrier.subresourceRange.levelCount = 1;
            barrier.subresourceRange.baseArrayLayer = 0;
            barrier.subresourceRange.layerCount = 6;

            VkPipelineStageFlags sourceStage;
            VkPipelineStageFlags destinationStage;

            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = 0;

            sourceStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

            vkCmdPipelineBarrier(
                commandBuffer,
                sourceStage,
                destinationStage,
                0,
                0,
                nullptr,
                0,
                nullptr,
                1,
                &barrier
            );

            endSingleTimeCommands(mainRenderCommandPool, commandBuffer);

            depthImage.views.push_back(createImageView(depthImage, j, 1));
        }

        setImageDebugObjectName(device, depthImage, "point light laryer");
        *GPUDescriptors
             [descriptorBindingsConfig[descriptorBindingIndex]
                  .globalDescriptorOffset
              + i]
                 .GPUImage = depthImage;
    }

    for (unsigned int i = 0; i < POINT_LIGHTS_NUMBER; ++i) {
        (*GPUDescriptors
              [descriptorBindingsConfig[descriptorBindingIndex]
                   .globalDescriptorOffset
               + i]
                  .GPUImage)
            .viewType = VK_IMAGE_VIEW_TYPE_CUBE;

        setImageDebugObjectName(
            device,
            *GPUDescriptors
                 [descriptorBindingsConfig[descriptorBindingIndex]
                      .globalDescriptorOffset
                  + i]
                     .GPUImage,
            "point light cube"
        );
        (*GPUDescriptors
              [descriptorBindingsConfig[descriptorBindingIndex]
                   .globalDescriptorOffset
               + i]
                  .GPUImage)
            .views.push_back(createImageView(
                *GPUDescriptors
                     [descriptorBindingsConfig[descriptorBindingIndex]
                          .globalDescriptorOffset
                      + i]
                         .GPUImage,
                0,
                6
            ));
    }
}

VkFormat CVulkanRenderer::findSupportedFormat(
    const std::vector<VkFormat>& candidates,
    VkImageTiling tiling,
    VkFormatFeatureFlags features
) {
    for (VkFormat format : candidates) {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

        if (tiling == VK_IMAGE_TILING_LINEAR
            && (props.linearTilingFeatures & features) == features) {
            return format;
        } else if (
            tiling == VK_IMAGE_TILING_OPTIMAL
            && (props.optimalTilingFeatures & features) == features
        ) {
            return format;
        }
    }

    throw std::runtime_error("failed to find supported format!");
}

VkFormat CVulkanRenderer::findDepthFormat() {
    return findSupportedFormat(
        {VK_FORMAT_D32_SFLOAT,
         VK_FORMAT_D32_SFLOAT_S8_UINT,
         VK_FORMAT_D24_UNORM_S8_UINT},
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
    );
}

bool CVulkanRenderer::hasStencilComponent(VkFormat format) {
    return format == VK_FORMAT_D32_SFLOAT_S8_UINT
        || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

void CVulkanRenderer::createTextureImageView() {
    unsigned int readableTextureDescriptorBindingIndex =
        descriptorSetsConfig[DescriptorSetDataLink::RIDABLE_TEXTURES]
            .descriptorsBindingsIDs[0];
    for (unsigned int i = 0; i < initializeTextureData_.size(); ++i) {
        VK_Image* image =
            GPUDescriptors
                [descriptorBindingsConfig[readableTextureDescriptorBindingIndex]
                     .globalDescriptorOffset
                 + i]
                    .GPUImage;
        image->views.push_back(createImageView(*image, 0, 1));
    }
}

void CVulkanRenderer::createTextureSampler() {
    VkPhysicalDeviceProperties properties {};
    vkGetPhysicalDeviceProperties(physicalDevice, &properties);

    VkSamplerCreateInfo samplerInfo {};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_NEAREST;
    samplerInfo.minFilter = VK_FILTER_NEAREST;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

    textureSampler = {}; /// TODO: Is it realy need here?
    if (vkCreateSampler(device, &samplerInfo, nullptr, &textureSampler)
        != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture sampler!");
    }
}

void CVulkanRenderer::createShadowMapSampler() {
    VkSamplerCreateInfo samplerInfo {};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;

    if (vkCreateSampler(device, &samplerInfo, nullptr, &shadowMapSampler)
        != VK_SUCCESS) {
        throw std::runtime_error("failed to create shadow map sampler!");
    }
}

VkImageView CVulkanRenderer::createImageView(
    VK_Image image,
    uint32_t baseArrayLayers,
    uint32_t layerCount
) {
    VkImageViewCreateInfo viewInfo {};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image.image;
    viewInfo.viewType = image.viewType;
    viewInfo.format = image.format;
    viewInfo.components.r = image.red;
    viewInfo.components.g = image.green;
    viewInfo.components.b = image.blue;
    viewInfo.components.a = image.alpha;
    viewInfo.subresourceRange.aspectMask = image.aspectFlags;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = baseArrayLayers;
    viewInfo.subresourceRange.layerCount = layerCount;

    VkImageView imageView;
    if (vkCreateImageView(device, &viewInfo, nullptr, &imageView)
        != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture image view!");
    }

    return imageView;
}

void CVulkanRenderer::createImage(VK_Image& image) {
    VkImageCreateInfo imageInfo {};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = image.width;
    imageInfo.extent.height = image.height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = image.arrayLayers;
    imageInfo.format = image.format;
    imageInfo.tiling = image.tiling;
    imageInfo.usage = image.usageFlags;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.flags = image.createFlags;

    if (vkCreateImage(device, &imageInfo, nullptr, &image.image)
        != VK_SUCCESS) {
        throw std::runtime_error("failed to create image!");
    }

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(device, image.image, &memRequirements);

    VkMemoryAllocateInfo allocInfo {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(
        memRequirements.memoryTypeBits,
        image.memoryPropertyFlags
    );

    if (vkAllocateMemory(device, &allocInfo, nullptr, &image.deviceMemory)
        != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate image memory!");
    }

    vkBindImageMemory(device, image.image, image.deviceMemory, 0);
}

void CVulkanRenderer::transitionImageLayout(
    VkImage image,
    VkImageLayout oldLayout,
    VkImageLayout newLayout
) {
    VkCommandBuffer commandBuffer =
        beginSingleTimeCommands(mainRenderCommandPool);

    VkImageMemoryBarrier barrier {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED
        && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if (
        oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
        && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    ) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else {
        throw std::invalid_argument("unsupported layout transition!");
    }

    vkCmdPipelineBarrier(
        commandBuffer,
        sourceStage,
        destinationStage,
        0,
        0,
        nullptr,
        0,
        nullptr,
        1,
        &barrier
    );

    endSingleTimeCommands(mainRenderCommandPool, commandBuffer);
}

void CVulkanRenderer::transitionShadowMapImageLayout(
    VkImage image,
    VkImageLayout oldLayout,
    VkImageLayout newLayout
) {
    VkCommandBuffer commandBuffer =
        beginSingleTimeCommands(directionalLightCommandPool);

    VkImageMemoryBarrier barrier {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED
        && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if (
        oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
        && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    ) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else {
        throw std::invalid_argument("unsupported layout transition!");
    }

    vkCmdPipelineBarrier(
        commandBuffer,
        sourceStage,
        destinationStage,
        0,
        0,
        nullptr,
        0,
        nullptr,
        1,
        &barrier
    );

    endSingleTimeCommands(directionalLightCommandPool, commandBuffer);
}

void CVulkanRenderer::copyBufferToImage(
    VkBuffer& buffer,
    VkImage image,
    uint32_t width,
    uint32_t height
) {
    VkCommandBuffer commandBuffer =
        beginSingleTimeCommands(mainRenderCommandPool);

    VkBufferImageCopy region {};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {width, height, 1};

    vkCmdCopyBufferToImage(
        commandBuffer,
        buffer,
        image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &region
    );

    endSingleTimeCommands(mainRenderCommandPool, commandBuffer);
}

void CVulkanRenderer::createVertexBuffer(
    VkBuffer& _vertexBuffer,
    VkDeviceMemory& _vertexBufferMemory,
    std::vector<Vertex>& _vertices
) {
    VkDeviceSize bufferSize = sizeof(_vertices[0]) * _vertices.size();
    if (_vertices.size() == 0) {
        std::cout << "warning: empty vertex mesh, skipping buffer copy"
                  << std::endl;
        bufferSize = 1;
    }

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(
        bufferSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
            | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        stagingBuffer,
        stagingBufferMemory
    );

    void* data;
    vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
    if (_vertices.size() > 0) {
        memcpy(data, _vertices.data(), (size_t)bufferSize);
    }
    vkUnmapMemory(device, stagingBufferMemory);

    createBuffer(
        bufferSize,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        _vertexBuffer,
        _vertexBufferMemory
    );

    copyBuffer(stagingBuffer, _vertexBuffer, bufferSize);

    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);
}

void CVulkanRenderer::createIndexBuffer(
    VkBuffer& _indexBuffer,
    VkDeviceMemory& _indexBufferMemory,
    const std::vector<uint32_t>& _indices
) {
    VkDeviceSize bufferSize = sizeof(_indices[0]) * _indices.size();
    if (_indices.empty()) {
        std::cout << "warning: empty index mesh, skipping buffer copy"
                  << std::endl;
        bufferSize = 1;
    }

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(
        bufferSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
            | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        stagingBuffer,
        stagingBufferMemory
    );

    void* data;
    vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
    if (!_indices.empty()) {
        memcpy(data, _indices.data(), (size_t)bufferSize);
    }
    vkUnmapMemory(device, stagingBufferMemory);

    createBuffer(
        bufferSize,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        _indexBuffer,
        _indexBufferMemory
    );

    copyBuffer(stagingBuffer, _indexBuffer, bufferSize);

    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);
}

void CVulkanRenderer::createMainRenderUniformBuffers() {
    for (unsigned int descriptorSetConfigCounter = 0; descriptorSetConfigCounter
         < DescriptorSetDataLink::DESCRIPTOR_CHUNKS_NUMBER;
         ++descriptorSetConfigCounter) {
        for (unsigned int j = 0;
             j < descriptorSetsConfig[descriptorSetConfigCounter]
                     .actualLinkedDescriptorBindingsNumber;
             ++j) {
            unsigned int descriptorBindingIndex =
                descriptorSetsConfig[descriptorSetConfigCounter]
                    .descriptorsBindingsIDs[j];
            VkDescriptorType descriptorType =
                descriptorBindingsConfig[descriptorBindingIndex].vkType;
            if (descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) {
                uint32_t memory =
                    descriptorBindingsConfig[descriptorBindingIndex].uboChunkSize
                    * descriptorSetsConfig[descriptorSetConfigCounter]
                          .hostDescriptorNumber;
                createBuffer(
                    memory,
                    VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
                        | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                    GPUDescriptors[descriptorBindingsConfig[descriptorBindingIndex]
                                       .globalDescriptorOffset]
                        .GPUBuffer->buffer,
                    GPUDescriptors[descriptorBindingsConfig[descriptorBindingIndex]
                                       .globalDescriptorOffset]
                        .GPUBuffer->deviceMemory
                );
            } else {
                continue;
            }
        }
    }
}

void CVulkanRenderer::createMainRenderDescriptorPool() {
    std::array<VkDescriptorPoolSize, 2> poolSizes {};

    uint32_t descriptorCount = 10000;
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = static_cast<uint32_t>(descriptorCount);
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = static_cast<uint32_t>(descriptorCount);

    VkDescriptorPoolCreateInfo poolInfo {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = static_cast<uint32_t>(descriptorCount);

    if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool)
        != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor pool!");
    }
}

void CVulkanRenderer::allocateDescriptorSets(
    std::vector<VkDescriptorSet>& descriptorSets,
    VkDescriptorSetLayout setLayout,
    const unsigned int descriptorSetsNumber,
    const unsigned int descriptorOffset
) {
    std::vector<VkDescriptorSetLayout> matrixUboLayouts(
        descriptorSetsNumber,
        setLayout
    );
    VkDescriptorSetAllocateInfo allocInfo {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(descriptorSetsNumber);
    allocInfo.pSetLayouts = matrixUboLayouts.data();
    if (vkAllocateDescriptorSets(
            device,
            &allocInfo,
            descriptorSets.data() + descriptorOffset
        )
        != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }
}

void CVulkanRenderer::updateDescriptorSetsUBO(
    VkBuffer ubo,
    const VkDeviceSize& uboStructSize,
    const unsigned int& uboDescriptorsNumber,
    int uboBinding,
    [[maybe_unused]] std::vector<VkDescriptorSet>& uboDescriptorSets,
    const unsigned int offset
) {
    for (size_t i = 0; i < uboDescriptorsNumber; ++i) {
        VkDescriptorBufferInfo modelMatrixBufferInfo =
            createDescriptorBufferInfo(ubo, uboStructSize, i);
        std::array<VkWriteDescriptorSet, 1> descriptorWrites {};

        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet =
            *(descriptorSetsChunks.data() + offset + i);
        descriptorWrites[0].dstBinding = uboBinding;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo = &modelMatrixBufferInfo;

        vkUpdateDescriptorSets(
            device,
            static_cast<uint32_t>(descriptorWrites.size()),
            descriptorWrites.data(),
            0,
            nullptr
        );
    }
}

void CVulkanRenderer::updateLightDataDescriptorSets(
    const DescriptorSet& currentDescriptorSet1
) {
    const unsigned int linkedDescriptorSetBindingsNumber =
        currentDescriptorSet1.actualLinkedDescriptorBindingsNumber;
    std::vector<uint32_t> shaderBindings;
    std::vector<uint32_t> descriptorNumberPerBinding;
    std::vector<uint32_t> bindingsIDs;
    for (size_t j = 0; j < linkedDescriptorSetBindingsNumber; ++j) {
        shaderBindings.push_back(
            descriptorBindingsConfig[currentDescriptorSet1
                                         .descriptorsBindingsIDs[j]]
                .binding
        );
        bindingsIDs.push_back(currentDescriptorSet1.descriptorsBindingsIDs[j]);
    }

    for (size_t i = 0; i < currentDescriptorSet1.hostDescriptorNumber; ++i) {
        std::vector<VkWriteDescriptorSet> descriptorWrites;
        descriptorWrites.resize(shaderBindings.size());
        std::vector<VkDescriptorBufferInfo> descriptorBufferInfos;
        std::vector<std::vector<VkDescriptorImageInfo>> descriptorImageInfos;
        for (size_t j = 0; j < shaderBindings.size(); ++j) {
            if (descriptorBindingsConfig[bindingsIDs[j]].vkType
                == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) {
                descriptorBufferInfos.push_back({});

                for (size_t m = 0; m < descriptorBindingsConfig[bindingsIDs[j]]
                                           .shaderDescriptorsNumber;
                     ++m) {
                    descriptorBufferInfos[j] = createDescriptorBufferInfo(
                        GPUDescriptors[descriptorBindingsConfig[bindingsIDs[j]]
                                           .globalDescriptorOffset]
                            .GPUBuffer->buffer,
                        descriptorBindingsConfig[bindingsIDs[j]].uboChunkSize,
                        i
                    );
                }
                descriptorWrites[j].pBufferInfo = descriptorBufferInfos.data();
            } else if (
                descriptorBindingsConfig[bindingsIDs[j]].vkType
                == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
            ) {
                descriptorImageInfos.push_back({});

                for (size_t m = 0; m < descriptorBindingsConfig[bindingsIDs[j]]
                                           .shaderDescriptorsNumber;
                     ++m) {
                    uint32_t imageViewIndex =
                        GPUDescriptors
                            [descriptorBindingsConfig[bindingsIDs[j]]
                                 .globalDescriptorOffset
                             + m]
                                .GPUImage->views.size()
                        - 1;

                    descriptorImageInfos[descriptorImageInfos.size() - 1]
                        .push_back({});
                    descriptorImageInfos[descriptorImageInfos.size() - 1][m] =
                        createDescriptorImageInfo(
                            *GPUDescriptors
                                 [descriptorBindingsConfig[bindingsIDs[j]]
                                      .globalDescriptorOffset
                                  + m]
                                     .GPUImage,
                            VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
                            imageViewIndex,
                            shadowMapSampler
                        );
                }
                descriptorWrites[j].pImageInfo =
                    descriptorImageInfos[descriptorImageInfos.size() - 1].data();
            }
            descriptorWrites[j].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[j].dstSet =
                *(descriptorSetsChunks.data()
                  + currentDescriptorSet1.descriptorSetOffset + i);
            descriptorWrites[j].dstBinding = shaderBindings[j];
            descriptorWrites[j].dstArrayElement = 0;
            descriptorWrites[j].descriptorType =
                descriptorBindingsConfig[bindingsIDs[j]].vkType;
            descriptorWrites[j].descriptorCount =
                descriptorBindingsConfig[bindingsIDs[j]].shaderDescriptorsNumber;
        }

        vkUpdateDescriptorSets(
            device,
            static_cast<uint32_t>(descriptorWrites.size()),
            descriptorWrites.data(),
            0,
            nullptr
        );
    }
}

void CVulkanRenderer::updateDescriptorSetsCombinedImageSampler(
    const DescriptorSet& descriptorSet
) {
    std::vector<uint32_t> bindingsIDs;
    for (size_t j = 0; j < descriptorSet.actualLinkedDescriptorBindingsNumber;
         ++j) {
        bindingsIDs.push_back(descriptorSet.descriptorsBindingsIDs[j]);
    }

    unsigned int readableTextureDescriptorBindingIndex =
        descriptorSetsConfig[DescriptorSetDataLink::RIDABLE_TEXTURES]
            .descriptorsBindingsIDs[0];
    for (size_t i = 0; i < descriptorSet.hostDescriptorNumber; ++i) {
        const unsigned int textureIndex = i / 2;
        constexpr unsigned int textureViewIndex = 0;
        VkDescriptorImageInfo imageInfo = createDescriptorImageInfo(
            *GPUDescriptors
                 [descriptorBindingsConfig[readableTextureDescriptorBindingIndex]
                      .globalDescriptorOffset
                  + textureIndex]
                     .GPUImage,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            textureViewIndex,
            textureSampler
        );
        std::vector<VkWriteDescriptorSet> descriptorWrites {};

        for (unsigned int j = 0; j < bindingsIDs.size(); ++j) {
            descriptorWrites.push_back({});
            const unsigned int lastElement = descriptorWrites.size() - 1;
            descriptorWrites[lastElement].sType =
                VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[lastElement].dstSet =
                *(descriptorSetsChunks.data()
                  + descriptorSet.descriptorSetOffset + i);
            descriptorWrites[lastElement].dstBinding =
                descriptorBindingsConfig[bindingsIDs[j]].binding;
            descriptorWrites[lastElement].dstArrayElement = 0;
            descriptorWrites[lastElement].descriptorType =
                VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorWrites[lastElement].descriptorCount = 1;
            descriptorWrites[lastElement].pImageInfo = &imageInfo;
        }
        vkUpdateDescriptorSets(
            device,
            static_cast<uint32_t>(descriptorWrites.size()),
            descriptorWrites.data(),
            0,
            nullptr
        );
    }
}

void CVulkanRenderer::createDescriptorImageInfo(
    const unsigned int descriptorNumber,
    VkImageLayout imageLayout,
    std::vector<VK_Image>& textureImages,
    const unsigned int imageViewIndex,
    VkDescriptorImageInfo descriptorImageInfos[]
) {
    for (size_t i = 0; i < descriptorNumber; ++i) {
        descriptorImageInfos[i] = {};
        descriptorImageInfos[i].imageLayout = imageLayout;
        descriptorImageInfos[i].imageView =
            textureImages[i].views[imageViewIndex];
        descriptorImageInfos[i].sampler = textureSampler;
    }
}

void CVulkanRenderer::createMainRenderDescriptorSets() {
    vkResetDescriptorPool(device, descriptorPool, 0);
    for (unsigned int pipelineCounter = 0;
         pipelineCounter < SpecificPipeline::PIPELINES_NUMBER;
         ++pipelineCounter) {
        for (unsigned int descriptorSetCounter = 0; descriptorSetCounter
             < pipelineConfigs[pipelineCounter].actualLinkedDescriptorSetsNumber;
             ++descriptorSetCounter) {
            const unsigned int linkedDescriptorSetMatrixUboID =
                pipelineConfigs[pipelineCounter]
                    .linkedDescriptorSetIDs[descriptorSetCounter];
            const DescriptorSet& currentDescriptorSet0 =
                descriptorSetsConfig[linkedDescriptorSetMatrixUboID];
            allocateDescriptorSets(
                descriptorSetsChunks,
                currentDescriptorSet0.setLayout,
                currentDescriptorSet0.hostDescriptorNumber,
                currentDescriptorSet0.descriptorSetOffset
            );
            if (currentDescriptorSet0.isTexture) {
                updateDescriptorSetsCombinedImageSampler(currentDescriptorSet0);
            } else {
                updateLightDataDescriptorSets(currentDescriptorSet0);
            }
        }
    }
}

void CVulkanRenderer::createBuffer(
    VkDeviceSize size,
    VkBufferUsageFlags usage,
    VkMemoryPropertyFlags properties,
    VkBuffer& buffer,
    VkDeviceMemory& bufferMemory
) {
    VkBufferCreateInfo bufferInfo {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    if (size == 0) {
        std::cout << "warning: createBuffer with size 0, clamped to 1"
                  << std::endl;
        size = 1;
    }
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to create buffer!");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex =
        findMemoryType(memRequirements.memoryTypeBits, properties);

    int32_t result =
        vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate buffer memory!");
    }

    vkBindBufferMemory(device, buffer, bufferMemory, 0);
}

VkCommandBuffer CVulkanRenderer::beginSingleTimeCommands(
    VkCommandPool& commandPool
) {
    VkCommandBufferAllocateInfo allocInfo {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}

void CVulkanRenderer::endSingleTimeCommands(
    VkCommandPool& commandPool,
    VkCommandBuffer& commandBuffer
) {
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(graphicsQueue);

    vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}

void CVulkanRenderer::copyBuffer(
    VkBuffer& srcBuffer,
    VkBuffer& dstBuffer,
    VkDeviceSize size
) {
    VkCommandBuffer commandBuffer =
        beginSingleTimeCommands(mainRenderCommandPool);

    VkBufferCopy copyRegion {};
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    endSingleTimeCommands(mainRenderCommandPool, commandBuffer);
}

uint32_t CVulkanRenderer::findMemoryType(
    uint32_t typeFilter,
    VkMemoryPropertyFlags properties
) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i))
            && (memProperties.memoryTypes[i].propertyFlags & properties)
                == properties) {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type!");
}

void CVulkanRenderer::createCommandBuffers(
    VkCommandPool& commandPool,
    std::vector<VkCommandBuffer>& commandBuffers,
    uint32_t commandBuffersNumber,
    VkCommandBufferLevel commandBufferLevelFlag
) {
    commandBuffers.resize(commandBuffersNumber * MAX_FRAMES_IN_FLIGHT);

    VkCommandBufferAllocateInfo allocInfo {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    allocInfo.level = commandBufferLevelFlag;
    allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

    if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data())
        != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate command buffers!");
    }
}

void CVulkanRenderer::executeSecondaryCommandBuffer(
    VkRenderPass renderPass,
    VkFramebuffer frameBuffer,
    VkExtent2D extent,
    VkCommandBuffer primaryCommandBuffer,
    VkCommandBuffer secondaryCommandBuffer
) {
    VkClearValue shadowMapClearValues[1];
    shadowMapClearValues[0].depthStencil.depth = 1.0f;
    shadowMapClearValues[0].depthStencil.stencil = 0;

    VkRenderPassBeginInfo shadowMapRenderPassInfo {};
    shadowMapRenderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    shadowMapRenderPassInfo.pNext = NULL;
    shadowMapRenderPassInfo.renderPass = renderPass;
    shadowMapRenderPassInfo.framebuffer = frameBuffer;
    shadowMapRenderPassInfo.renderArea.offset.x = 0;
    shadowMapRenderPassInfo.renderArea.offset.y = 0;
    shadowMapRenderPassInfo.renderArea.extent.width = extent.width;
    shadowMapRenderPassInfo.renderArea.extent.height = extent.height;
    shadowMapRenderPassInfo.clearValueCount = 1;
    shadowMapRenderPassInfo.pClearValues = shadowMapClearValues;

    vkCmdBeginRenderPass(
        primaryCommandBuffer,
        &shadowMapRenderPassInfo,
        VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS
    );
    vkCmdExecuteCommands(primaryCommandBuffer, 1, &secondaryCommandBuffer);
    vkCmdEndRenderPass(primaryCommandBuffer);
}

void CVulkanRenderer::updateHudUBO(
    uint32_t offset,
    bool isHudExists,
    float highestY,
    uint32_t healthCounter
) {
    HUD_UBO hudUBO {};

    hudUBO.view = viewMatrix;
    hudUBO.proj = projectionMatrix;

    hudUBO.isHudExists = isHudExists;
    hudUBO.currentHP = healthBars[healthCounter].currentHealth;
    hudUBO.maxHP = healthBars[healthCounter].maxHealth;
    hudUBO.entityPosition = healthBars[healthCounter].position;
    hudUBO.highestY = highestY;

    void* hudMatrixData;
    unsigned int hudUboDescriptorBindingIndex =
        descriptorSetsConfig[DescriptorSetDataLink::HUD]
            .descriptorsBindingsIDs[0];
    vkMapMemory(
        device,
        GPUDescriptors[descriptorBindingsConfig[hudUboDescriptorBindingIndex]
                           .globalDescriptorOffset]
            .GPUBuffer->deviceMemory,
        sizeof(HUD_UBO) * offset,
        sizeof(HUD_UBO),
        0,
        &hudMatrixData
    );
    memcpy(hudMatrixData, &hudUBO, sizeof(HUD_UBO));
    vkUnmapMemory(
        device,
        GPUDescriptors[descriptorBindingsConfig[hudUboDescriptorBindingIndex]
                           .globalDescriptorOffset]
            .GPUBuffer->deviceMemory
    );
}

void CVulkanRenderer::updateHudScreenUBO(uint32_t offset, uint32_t crosshair) {
    HUD_SCREEN_UBO hudUBO {};
    hudUBO.model = crosshairs[crosshair].model;

    void* hudMatrixData;
    unsigned int hudScreenUboDescriptorBindingIndex =
        descriptorSetsConfig[DescriptorSetDataLink::HUD_SCREEN]
            .descriptorsBindingsIDs[0];
    vkMapMemory(
        device,
        GPUDescriptors[descriptorBindingsConfig[hudScreenUboDescriptorBindingIndex]
                           .globalDescriptorOffset]
            .GPUBuffer->deviceMemory,
        sizeof(HUD_SCREEN_UBO) * offset,
        sizeof(HUD_SCREEN_UBO),
        0,
        &hudMatrixData
    );
    memcpy(hudMatrixData, &hudUBO, sizeof(HUD_SCREEN_UBO));
    vkUnmapMemory(
        device,
        GPUDescriptors[descriptorBindingsConfig[hudScreenUboDescriptorBindingIndex]
                           .globalDescriptorOffset]
            .GPUBuffer->deviceMemory
    );
}

void CVulkanRenderer::updateSdfUBO(uint32_t offset, uint32_t crosshair) {
    SDF_UBO hudUBO {};
    hudUBO.model = crosshairs[crosshair].model;

    float currentTime = std::chrono::duration_cast<std::chrono::milliseconds>(
                            std::chrono::steady_clock::now() - startTime
                        )
                            .count()
        * 0.001;
    hudUBO.iTime = currentTime;

    void* hudMatrixData;
    unsigned int hudScreenUboDescriptorBindingIndex =
        descriptorSetsConfig[DescriptorSetDataLink::SDF_DATA]
            .descriptorsBindingsIDs[0];
    vkMapMemory(
        device,
        GPUDescriptors[descriptorBindingsConfig[hudScreenUboDescriptorBindingIndex]
                           .globalDescriptorOffset]
            .GPUBuffer->deviceMemory,
        sizeof(SDF_UBO) * offset,
        sizeof(SDF_UBO),
        0,
        &hudMatrixData
    );
    memcpy(hudMatrixData, &hudUBO, sizeof(SDF_UBO));
    vkUnmapMemory(
        device,
        GPUDescriptors[descriptorBindingsConfig[hudScreenUboDescriptorBindingIndex]
                           .globalDescriptorOffset]
            .GPUBuffer->deviceMemory
    );
}

void CVulkanRenderer::updateUBO_UI(
    const unsigned int currentInventoryRow,
    const unsigned int currentInventoryColumn,
    const unsigned int inventory,
    uint32_t offset
) {
    UI_UBO hudUBO {};

    const unsigned int colSize = inventories[inventory].col;
    hudUBO.model =
        inventories[inventory]
            .slotData[colSize * currentInventoryRow + currentInventoryColumn]
            .model;
    hudUBO.color =
        inventories[inventory]
            .slotData[colSize * currentInventoryRow + currentInventoryColumn]
            .color;

    void* hudMatrixData;
    unsigned int uiUboDescriptorBindingIndex =
        descriptorSetsConfig[DescriptorSetDataLink::UI]
            .descriptorsBindingsIDs[0];
    vkMapMemory(
        device,
        GPUDescriptors[descriptorBindingsConfig[uiUboDescriptorBindingIndex]
                           .globalDescriptorOffset]
            .GPUBuffer->deviceMemory,
        sizeof(UI_UBO) * offset,
        sizeof(UI_UBO),
        0,
        &hudMatrixData
    );
    memcpy(hudMatrixData, &hudUBO, sizeof(UI_UBO));
    vkUnmapMemory(
        device,
        GPUDescriptors[descriptorBindingsConfig[uiUboDescriptorBindingIndex]
                           .globalDescriptorOffset]
            .GPUBuffer->deviceMemory
    );
}

void CVulkanRenderer::updateUBO_IconsUI(uint32_t offset, uint32_t item) {
    UI_UBO hudUBO {};
    hudUBO.model = items[item].model;

    void* hudMatrixData;
    unsigned int uiIconsUboDescriptorBindingIndex =
        descriptorSetsConfig[DescriptorSetDataLink::UI_ICONS]
            .descriptorsBindingsIDs[0];
    vkMapMemory(
        device,
        GPUDescriptors[descriptorBindingsConfig[uiIconsUboDescriptorBindingIndex]
                           .globalDescriptorOffset]
            .GPUBuffer->deviceMemory,
        sizeof(UI_UBO) * offset,
        sizeof(UI_UBO),
        0,
        &hudMatrixData
    );
    memcpy(hudMatrixData, &hudUBO, sizeof(UI_UBO));
    vkUnmapMemory(
        device,
        GPUDescriptors[descriptorBindingsConfig[uiIconsUboDescriptorBindingIndex]
                           .globalDescriptorOffset]
            .GPUBuffer->deviceMemory
    );
}

void CVulkanRenderer::hudRecordCommandBuffer(
    VkCommandBuffer& commandBuffer,
    uint32_t imageIndex
) {
    VkCommandBufferBeginInfo beginInfo {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    VkRenderPassBeginInfo renderPassInfo {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderPasses[SpecificPipeline::HUD_PIPELINE];
    renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent.height = swapChainExtent.height;
    renderPassInfo.renderArea.extent.width = swapChainExtent.width;

    std::array<VkClearValue, 2> clearValues {};
    clearValues[0].color = {{0.5f, 0.2f, 0.2f, 1.0f}};
    clearValues[1].depthStencil = {1.0f, 0};

    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(
        commandBuffer,
        &renderPassInfo,
        VK_SUBPASS_CONTENTS_INLINE
    );

    vkCmdBindPipeline(
        commandBuffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        pipelineConfigs[SpecificPipeline::HUD_PIPELINE].pipeline
    );

    VkViewport viewport {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)swapChainExtent.width;
    viewport.height = (float)swapChainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor {};
    scissor.offset = {0, 0};
    scissor.extent = swapChainExtent;
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    for (unsigned int i = 0; i < healthBars.size(); ++i) {
        unsigned int uiVertexId = healthBars[i].meshID;
        unsigned int uboIndex = currentFrame * hudUboDescriptorNumber + i;
        updateHudUBO(uboIndex, true, highest_gltf_Y[uiVertexId], i);
        const unsigned int linkedDescriptorSetID =
            pipelineConfigs[SpecificPipeline::HUD_PIPELINE]
                .linkedDescriptorSetIDs[0];
        const DescriptorSet& currentDescriptorSet =
            descriptorSetsConfig[linkedDescriptorSetID];
        vkCmdBindDescriptorSets(
            commandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            pipelineConfigs[SpecificPipeline::HUD_PIPELINE].pipelineLayout,
            0,
            1,
            &(*(descriptorSetsChunks.data()
                + currentDescriptorSet.descriptorSetOffset + i)),
            0,
            nullptr
        );

        VkBuffer vertexBuffers[] = {vertexBufferContainer[uiVertexId]};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

        vkCmdBindIndexBuffer(
            commandBuffer,
            indexBufferContainer[uiVertexId],
            0,
            VK_INDEX_TYPE_UINT32
        );

        unsigned int indicesContainerSize = aIndices_[uiVertexId].size();

        vkCmdDrawIndexed(
            commandBuffer,
            static_cast<uint32_t>(indicesContainerSize),
            1,
            0,
            0,
            0
        );
    }

    vkCmdEndRenderPass(commandBuffer);
}

void CVulkanRenderer::uiRecordCommandBuffer(
    VkCommandBuffer& commandBuffer,
    uint32_t imageIndex
) {
    VkCommandBufferBeginInfo beginInfo {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    VkRenderPassBeginInfo renderPassInfo {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderPasses[SpecificPipeline::UI_PIPELINE];
    renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent.height = swapChainExtent.height;
    renderPassInfo.renderArea.extent.width = swapChainExtent.width;

    std::array<VkClearValue, 2> clearValues {};
    clearValues[0].color = {{0.5f, 0.2f, 0.2f, 1.0f}};
    clearValues[1].depthStencil = {1.0f, 0};

    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(
        commandBuffer,
        &renderPassInfo,
        VK_SUBPASS_CONTENTS_INLINE
    );

    vkCmdBindPipeline(
        commandBuffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        pipelineConfigs[SpecificPipeline::UI_PIPELINE].pipeline
    );

    VkViewport viewport {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)swapChainExtent.width;
    viewport.height = (float)swapChainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor {};
    scissor.offset = {0, 0};
    scissor.extent = swapChainExtent;
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    for (unsigned int i = 0; i < inventories.size(); ++i) {
        RenderInventory inventory = inventories[i];
        unsigned int inventoryTextureID = inventory.inventoryTextureID;
        unsigned int uiVertexId = inventory.meshID;
        for (unsigned int j = 0; j < inventory.row; ++j) {
            for (unsigned int m = 0; m < inventory.col; ++m) {
                unsigned int uboIndex = currentFrame * uiUboDescriptorsNumber
                    + j * inventory.col + m;
                updateUBO_UI(j, m, i, uboIndex);
                const unsigned int linkedDescriptorSetID =
                    pipelineConfigs[SpecificPipeline::UI_PIPELINE]
                        .linkedDescriptorSetIDs[0];
                const DescriptorSet& currentDescriptorSet =
                    descriptorSetsConfig[linkedDescriptorSetID];
                vkCmdBindDescriptorSets(
                    commandBuffer,
                    VK_PIPELINE_BIND_POINT_GRAPHICS,
                    pipelineConfigs[SpecificPipeline::UI_PIPELINE]
                        .pipelineLayout,
                    0,
                    1,
                    &(*(descriptorSetsChunks.data()
                        + currentDescriptorSet.descriptorSetOffset + uboIndex)),
                    0,
                    nullptr
                );

                const unsigned int linkedDescriptorSetID1 =
                    pipelineConfigs[SpecificPipeline::UI_PIPELINE]
                        .linkedDescriptorSetIDs[1];
                const DescriptorSet& currentDescriptorSet1 =
                    descriptorSetsConfig[linkedDescriptorSetID1];
                vkCmdBindDescriptorSets(
                    commandBuffer,
                    VK_PIPELINE_BIND_POINT_GRAPHICS,
                    pipelineConfigs[SpecificPipeline::UI_PIPELINE]
                        .pipelineLayout,
                    1,
                    1,
                    &(*(descriptorSetsChunks.data()
                        + currentDescriptorSet1.descriptorSetOffset
                        + MAX_FRAMES_IN_FLIGHT * inventoryTextureID
                        + currentFrame)),
                    0,
                    nullptr
                );

                VkBuffer vertexBuffers[] = {vertexBufferContainer[uiVertexId]};
                VkDeviceSize offsets[] = {0};
                vkCmdBindVertexBuffers(
                    commandBuffer,
                    0,
                    1,
                    vertexBuffers,
                    offsets
                );

                vkCmdBindIndexBuffer(
                    commandBuffer,
                    indexBufferContainer[uiVertexId],
                    0,
                    VK_INDEX_TYPE_UINT32
                );

                unsigned int indicesContainerSize =
                    aIndices_[uiVertexId].size();

                vkCmdDrawIndexed(
                    commandBuffer,
                    static_cast<uint32_t>(indicesContainerSize),
                    1,
                    0,
                    0,
                    0
                );
            }
        }
    }

    vkCmdEndRenderPass(commandBuffer);
}

void CVulkanRenderer::uiIconsRecordCommandBuffer(
    VkCommandBuffer& commandBuffer,
    uint32_t imageIndex
) {
    VkCommandBufferBeginInfo beginInfo {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    VkRenderPassBeginInfo renderPassInfo {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass =
        renderPasses[SpecificPipeline::UI_ICONS_PIPELINE];
    renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent.height = swapChainExtent.height;
    renderPassInfo.renderArea.extent.width = swapChainExtent.width;

    std::array<VkClearValue, 2> clearValues {};
    clearValues[0].color = {{0.5f, 0.2f, 0.2f, 1.0f}};
    clearValues[1].depthStencil = {1.0f, 0};

    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(
        commandBuffer,
        &renderPassInfo,
        VK_SUBPASS_CONTENTS_INLINE
    );

    vkCmdBindPipeline(
        commandBuffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        pipelineConfigs[SpecificPipeline::UI_ICONS_PIPELINE].pipeline
    );

    VkViewport viewport {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)swapChainExtent.width;
    viewport.height = (float)swapChainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor {};
    scissor.offset = {0, 0};
    scissor.extent = swapChainExtent;
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    for (unsigned int i = 0; i < items.size(); ++i) {
        RenderItem item = items[i];
        unsigned int uiVertexId = item.meshID;
        unsigned int diffuseTextureID = item.diffuseTextureID;
        unsigned int uboIndex = currentFrame * items.size() + i;

        updateUBO_IconsUI(uboIndex, i);
        const unsigned int linkedDescriptorSetID =
            pipelineConfigs[SpecificPipeline::UI_ICONS_PIPELINE]
                .linkedDescriptorSetIDs[0];
        const DescriptorSet& currentDescriptorSet =
            descriptorSetsConfig[linkedDescriptorSetID];
        vkCmdBindDescriptorSets(
            commandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            pipelineConfigs[SpecificPipeline::UI_ICONS_PIPELINE].pipelineLayout,
            0,
            1,
            &(*(descriptorSetsChunks.data()
                + currentDescriptorSet.descriptorSetOffset + uboIndex)),
            0,
            nullptr
        );

        const unsigned int linkedDescriptorSetID1 =
            pipelineConfigs[SpecificPipeline::UI_ICONS_PIPELINE]
                .linkedDescriptorSetIDs[1];
        const DescriptorSet& currentDescriptorSet1 =
            descriptorSetsConfig[linkedDescriptorSetID1];
        vkCmdBindDescriptorSets(
            commandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            pipelineConfigs[SpecificPipeline::UI_ICONS_PIPELINE].pipelineLayout,
            1,
            1,
            &(*(descriptorSetsChunks.data()
                + currentDescriptorSet1.descriptorSetOffset
                + MAX_FRAMES_IN_FLIGHT * diffuseTextureID + currentFrame)),
            0,
            nullptr
        );

        VkBuffer vertexBuffers[] = {vertexBufferContainer[uiVertexId]};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

        vkCmdBindIndexBuffer(
            commandBuffer,
            indexBufferContainer[uiVertexId],
            0,
            VK_INDEX_TYPE_UINT32
        );

        unsigned int indicesContainerSize = aIndices_[uiVertexId].size();

        vkCmdDrawIndexed(
            commandBuffer,
            static_cast<uint32_t>(indicesContainerSize),
            1,
            0,
            0,
            0
        );
    }

    vkCmdEndRenderPass(commandBuffer);
}

void CVulkanRenderer::hudScreenRecordCommandBuffer(
    VkCommandBuffer& commandBuffer,
    uint32_t imageIndex
) {
    VkCommandBufferBeginInfo beginInfo {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    VkRenderPassBeginInfo renderPassInfo {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass =
        renderPasses[SpecificPipeline::HUD_SCREEN_PIPELINE];
    renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent.height = swapChainExtent.height;
    renderPassInfo.renderArea.extent.width = swapChainExtent.width;

    std::array<VkClearValue, 2> clearValues {};
    clearValues[0].color = {{0.5f, 0.2f, 0.2f, 1.0f}};
    clearValues[1].depthStencil = {1.0f, 0};

    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(
        commandBuffer,
        &renderPassInfo,
        VK_SUBPASS_CONTENTS_INLINE
    );

    vkCmdBindPipeline(
        commandBuffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        pipelineConfigs[SpecificPipeline::HUD_SCREEN_PIPELINE].pipeline
    );

    VkViewport viewport {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)swapChainExtent.width;
    viewport.height = (float)swapChainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor {};
    scissor.offset = {0, 0};
    scissor.extent = swapChainExtent;
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    for (unsigned int i = 0; i < crosshairs.size(); ++i) {
        RenderCrosshair crosshair = crosshairs[i];
        unsigned int uiVertexId = crosshair.meshID;

        unsigned int uboIndex = currentFrame * hudScreenUboDescriptorNumber + i;
        updateHudScreenUBO(uboIndex, i);
        const unsigned int linkedDescriptorSetID =
            pipelineConfigs[SpecificPipeline::HUD_SCREEN_PIPELINE]
                .linkedDescriptorSetIDs[0];
        const DescriptorSet& currentDescriptorSet =
            descriptorSetsConfig[linkedDescriptorSetID];
        vkCmdBindDescriptorSets(
            commandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            pipelineConfigs[SpecificPipeline::HUD_SCREEN_PIPELINE]
                .pipelineLayout,
            0,
            1,
            &(*(descriptorSetsChunks.data()
                + currentDescriptorSet.descriptorSetOffset + uboIndex)),
            0,
            nullptr
        );

        VkBuffer vertexBuffers[] = {vertexBufferContainer[uiVertexId]};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

        vkCmdBindIndexBuffer(
            commandBuffer,
            indexBufferContainer[uiVertexId],
            0,
            VK_INDEX_TYPE_UINT32
        );

        unsigned int indicesContainerSize = aIndices_[uiVertexId].size();

        vkCmdDrawIndexed(
            commandBuffer,
            static_cast<uint32_t>(indicesContainerSize),
            1,
            0,
            0,
            0
        );
    }

    vkCmdEndRenderPass(commandBuffer);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to record command buffer!");
    }
}

void CVulkanRenderer::sdfRecordCommandBuffer(
    VkCommandBuffer& commandBuffer,
    uint32_t imageIndex
) {
    VkCommandBufferBeginInfo beginInfo {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    VkRenderPassBeginInfo renderPassInfo {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderPasses[SpecificPipeline::SDF_PIPELINE];
    renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent.height = swapChainExtent.height;
    renderPassInfo.renderArea.extent.width = swapChainExtent.width;

    std::array<VkClearValue, 2> clearValues {};
    clearValues[0].color = {{0.5f, 0.2f, 0.2f, 1.0f}};
    clearValues[1].depthStencil = {1.0f, 0};

    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(
        commandBuffer,
        &renderPassInfo,
        VK_SUBPASS_CONTENTS_INLINE
    );

    vkCmdBindPipeline(
        commandBuffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        pipelineConfigs[SpecificPipeline::SDF_PIPELINE].pipeline
    );

    VkViewport viewport {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)swapChainExtent.width;
    viewport.height = (float)swapChainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor {};
    scissor.offset = {0, 0};
    scissor.extent = swapChainExtent;
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    for (unsigned int i = 0; i < crosshairs.size(); ++i) {
        RenderCrosshair crosshair = crosshairs[i];
        unsigned int uiVertexId = crosshair.meshID;

        unsigned int uboIndex = currentFrame * hudScreenUboDescriptorNumber + i;
        updateSdfUBO(uboIndex, i);
        const unsigned int linkedDescriptorSetID =
            pipelineConfigs[SpecificPipeline::SDF_PIPELINE]
                .linkedDescriptorSetIDs[0];
        const DescriptorSet& currentDescriptorSet =
            descriptorSetsConfig[linkedDescriptorSetID];
        vkCmdBindDescriptorSets(
            commandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            pipelineConfigs[SpecificPipeline::SDF_PIPELINE].pipelineLayout,
            0,
            1,
            &(*(descriptorSetsChunks.data()
                + currentDescriptorSet.descriptorSetOffset + uboIndex)),
            0,
            nullptr
        );

        VkBuffer vertexBuffers[] = {vertexBufferContainer[uiVertexId]};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

        vkCmdBindIndexBuffer(
            commandBuffer,
            indexBufferContainer[uiVertexId],
            0,
            VK_INDEX_TYPE_UINT32
        );
        vkCmdDrawIndexed(commandBuffer, 3, 1, 0, 0, 0);
    }

    vkCmdEndRenderPass(commandBuffer);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to record command buffer!");
    }
}

void CVulkanRenderer::fontRecordCommandBuffer(
    VkCommandBuffer& commandBuffer,
    uint32_t imageIndex
) {
    VkCommandBufferBeginInfo beginInfo {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    VkRenderPassBeginInfo renderPassInfo {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderPasses[SpecificPipeline::FONT_PIPELINE];
    renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent.height = swapChainExtent.height;
    renderPassInfo.renderArea.extent.width = swapChainExtent.width;
    std::array<VkClearValue, 2> clearValues {};
    clearValues[0].color = {{0.5f, 0.2f, 0.2f, 1.0f}};
    clearValues[1].depthStencil = {1.0f, 0};

    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(
        commandBuffer,
        &renderPassInfo,
        VK_SUBPASS_CONTENTS_INLINE
    );

    vkCmdBindPipeline(
        commandBuffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        pipelineConfigs[SpecificPipeline::FONT_PIPELINE].pipeline
    );

    VkViewport viewport {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)swapChainExtent.width;
    viewport.height = (float)swapChainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor {};
    scissor.offset = {0, 0};
    scissor.extent = swapChainExtent;
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
    for (unsigned int playerCounter = 0; playerCounter < players.size();
         ++playerCounter) {
        player = players[playerCounter];
    }
    unsigned int currentActorMemoryOffset =
        currentFrame * fontUboDescriptorNumber;
    for (unsigned int i = 0; i < fonts.size(); ++i) {
        RenderFont font = fonts[i];
        Vector<float, 3> playerTragetDirection =
            font.position - player.position;
        float dotProduct = Dot(playerTragetDirection, player.forward);
        if (dotProduct <= 0) {
            continue;
        }

        for (unsigned int j = 0; j < font.font_string.size(); ++j) {
            unsigned int ascii_code =
                static_cast<unsigned int>(font.font_string[j]);
            VkBuffer vertexBuffers[] = {fontVertexBufferContainer[ascii_code]};
            VkDeviceSize offsets[] = {0};

            vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
            vkCmdBindIndexBuffer(
                commandBuffer,
                fontIndexBufferContainer[ascii_code],
                0,
                VK_INDEX_TYPE_UINT32
            );

            unsigned int indicesContainerSize = symbol_g_indices.size();
            FONT_UBO fontUBO {};
            Vector<float, 3> result;
            Vector<float, 4> pos = Vector<float, 4>(
                font.position[0],
                font.position[1],
                font.position[2],
                1.0f
            );

            Vector<float, 4> clipSpacePosition =
                pos * viewMatrix * projectionMatrix;
            Vector<float, 3> ndcPosition = Vector<float, 3>(
                clipSpacePosition[0] / clipSpacePosition[3],
                clipSpacePosition[1] / clipSpacePosition[3],
                clipSpacePosition[2] / clipSpacePosition[3]
            );

            fontUBO.view = viewMatrix;
            fontUBO.proj = projectionMatrix;

            fontUBO.scale = 0.3f;
            ndcPosition[0] += (float)j * 0.17f * fontUBO.scale;
            ndcPosition[1] -= font.lifeTime / 5.0f;
            fontUBO.position = ndcPosition;

            void* modelMatrixData;
            unsigned int fontUboDescriptorBindingIndex =
                descriptorSetsConfig[DescriptorSetDataLink::FONT_RENDER_UBO]
                    .descriptorsBindingsIDs[0];
            vkMapMemory(
                device,
                GPUDescriptors
                    [descriptorBindingsConfig[fontUboDescriptorBindingIndex]
                         .globalDescriptorOffset]
                        .GPUBuffer->deviceMemory,
                sizeof(fontUBO) * (currentActorMemoryOffset + j),
                sizeof(fontUBO),
                0,
                &modelMatrixData
            );
            memcpy(modelMatrixData, &fontUBO, sizeof(fontUBO));
            vkUnmapMemory(
                device,
                GPUDescriptors
                    [descriptorBindingsConfig[fontUboDescriptorBindingIndex]
                         .globalDescriptorOffset]
                        .GPUBuffer->deviceMemory
            );

            const unsigned int linkedDescriptorSetID =
                pipelineConfigs[SpecificPipeline::FONT_PIPELINE]
                    .linkedDescriptorSetIDs[0];
            const DescriptorSet& currentDescriptorSet =
                descriptorSetsConfig[linkedDescriptorSetID];
            vkCmdBindDescriptorSets(
                commandBuffer,
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                pipelineConfigs[SpecificPipeline::FONT_PIPELINE].pipelineLayout,
                0,
                1,
                &(*(descriptorSetsChunks.data()
                    + currentDescriptorSet.descriptorSetOffset
                    + currentActorMemoryOffset + j)),
                0,
                nullptr
            );
            const unsigned int linkedDescriptorSetID1 =
                pipelineConfigs[SpecificPipeline::FONT_PIPELINE]
                    .linkedDescriptorSetIDs[1];
            const unsigned int fontAtlasTextureID = 6;
            const DescriptorSet& currentDescriptorSet1 =
                descriptorSetsConfig[linkedDescriptorSetID1];
            vkCmdBindDescriptorSets(
                commandBuffer,
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                pipelineConfigs[SpecificPipeline::FONT_PIPELINE].pipelineLayout,
                1,
                1,
                &(*(descriptorSetsChunks.data()
                    + currentDescriptorSet1.descriptorSetOffset
                    + MAX_FRAMES_IN_FLIGHT * fontAtlasTextureID
                    + currentFrame)),
                0,
                nullptr
            );

            vkCmdDrawIndexed(
                commandBuffer,
                static_cast<uint32_t>(indicesContainerSize),
                1,
                0,
                0,
                0
            );
        }
        currentActorMemoryOffset += font.font_string.size();
    }
    vkCmdEndRenderPass(commandBuffer);
}

void CVulkanRenderer::recordCommandBuffer(
    VkCommandBuffer& commandBuffer,
    uint32_t imageIndex
) {
    VkRenderPassBeginInfo renderPassInfo {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass =
        renderPasses[SpecificPipeline::MAIN_RENDER_PIPELINE];
    renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent.height = swapChainExtent.height;
    renderPassInfo.renderArea.extent.width = swapChainExtent.width;

    for (unsigned int player = 0; player < players.size(); ++player) {
        updateViewPositionUniformBuffer(currentFrame, player);
    }

    std::array<VkClearValue, 2> clearValues {};
    // Player death screen.
    if (players.size() == 0) {
        clearValues[0].color = {{0.7f, 0.2f, 0.2f, 1.0f}};
    } else {
        clearValues[0].color = {{0.2f, 0.2f, 0.2f, 1.0f}};
    }
    clearValues[1].depthStencil = {1.0f, 0};

    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(
        commandBuffer,
        &renderPassInfo,
        VK_SUBPASS_CONTENTS_INLINE
    );

    vkCmdBindPipeline(
        commandBuffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        pipelineConfigs[SpecificPipeline::MAIN_RENDER_PIPELINE].pipeline
    );

    VkViewport viewport {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)swapChainExtent.width;
    viewport.height = (float)swapChainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor {};
    scissor.offset = {0, 0};
    scissor.extent = swapChainExtent;
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    for (unsigned int i = 0; i < actors.size(); ++i) {
        RenderActor actor = actors[i];
        unsigned int uiVertexId = actor.meshID;
        unsigned int diffuseTextureIndex = actor.diffuseTextureIndex;
        unsigned int specularTextureIndex = actor.specularTextureIndex;

        unsigned int uboIndex = currentFrame * matrixUboDescriptorsNumber + i;
        updateMatrixUniformBuffer(uboIndex, i);
        const unsigned int linkedDescriptorSetID =
            pipelineConfigs[SpecificPipeline::MAIN_RENDER_PIPELINE]
                .linkedDescriptorSetIDs[0];
        const DescriptorSet& currentDescriptorSet =
            descriptorSetsConfig[linkedDescriptorSetID];
        vkCmdBindDescriptorSets(
            commandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            pipelineConfigs[SpecificPipeline::MAIN_RENDER_PIPELINE]
                .pipelineLayout,
            0,
            1,
            &(*(descriptorSetsChunks.data()
                + currentDescriptorSet.descriptorSetOffset + uboIndex)),
            0,
            nullptr
        );

        const unsigned int linkedDescriptorSetID1 =
            pipelineConfigs[SpecificPipeline::MAIN_RENDER_PIPELINE]
                .linkedDescriptorSetIDs[1];
        const DescriptorSet& currentDescriptorSet1 =
            descriptorSetsConfig[linkedDescriptorSetID1];
        vkCmdBindDescriptorSets(
            commandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            pipelineConfigs[SpecificPipeline::MAIN_RENDER_PIPELINE]
                .pipelineLayout,
            1,
            1,
            &(*(descriptorSetsChunks.data()
                + currentDescriptorSet1.descriptorSetOffset + currentFrame)),
            0,
            nullptr
        );

        VkBuffer vertexBuffers[] = {vertexBufferContainer[uiVertexId]};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

        vkCmdBindIndexBuffer(
            commandBuffer,
            indexBufferContainer[uiVertexId],
            0,
            VK_INDEX_TYPE_UINT32
        );

        unsigned int indicesContainerSize = aIndices_[uiVertexId].size();

        const unsigned int linkedDescriptorSetID2 =
            pipelineConfigs[SpecificPipeline::MAIN_RENDER_PIPELINE]
                .linkedDescriptorSetIDs[2];
        const DescriptorSet& currentDescriptorSet2 =
            descriptorSetsConfig[linkedDescriptorSetID2];
        vkCmdBindDescriptorSets(
            commandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            pipelineConfigs[SpecificPipeline::MAIN_RENDER_PIPELINE]
                .pipelineLayout,
            2,
            1,
            &(*(descriptorSetsChunks.data()
                + currentDescriptorSet2.descriptorSetOffset
                + MAX_FRAMES_IN_FLIGHT * specularTextureIndex + currentFrame)),
            0,
            nullptr
        );
        const unsigned int linkedDescriptorSetID3 =
            pipelineConfigs[SpecificPipeline::MAIN_RENDER_PIPELINE]
                .linkedDescriptorSetIDs[3];
        const DescriptorSet& currentDescriptorSet3 =
            descriptorSetsConfig[linkedDescriptorSetID3];
        vkCmdBindDescriptorSets(
            commandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            pipelineConfigs[SpecificPipeline::MAIN_RENDER_PIPELINE]
                .pipelineLayout,
            3,
            1,
            &(*(descriptorSetsChunks.data()
                + currentDescriptorSet3.descriptorSetOffset
                + MAX_FRAMES_IN_FLIGHT * diffuseTextureIndex + currentFrame)),
            0,
            nullptr
        );

        vkCmdDrawIndexed(
            commandBuffer,
            static_cast<uint32_t>(indicesContainerSize),
            1,
            0,
            0,
            0
        );
    }
    vkCmdEndRenderPass(commandBuffer);
}

void CVulkanRenderer::createSyncObjects(
    std::vector<VkSemaphore>& imageAvailableSemaphores,
    std::vector<VkSemaphore>& renderFinishedSemaphores,
    std::vector<VkFence>& inFlightFences
) {
    imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    renderFinishedSemaphores.resize(swapChainImages.size());
    inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

    VkSemaphoreCreateInfo semaphoreInfo {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
        if (vkCreateSemaphore(
                device,
                &semaphoreInfo,
                nullptr,
                &imageAvailableSemaphores[i]
            ) != VK_SUCCESS
            || vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i])
                != VK_SUCCESS) {
            throw std::runtime_error(
                "failed to create synchronization objects for a frame!"
            );
        }
    }

    for (size_t i = 0; i < swapChainImages.size(); ++i) {
        if (vkCreateSemaphore(
                device,
                &semaphoreInfo,
                nullptr,
                &renderFinishedSemaphores[i]
            )
            != VK_SUCCESS) {
            throw std::runtime_error(
                "failed to create synchronization objects for a frame!"
            );
        }
    }
}

void CVulkanRenderer::updateDirectionalLightShadowMapMatrixUBO(
    uint32_t currentImage,
    uint32_t currentLight,
    unsigned int actor
) {
    ShadowMapMatrixUBO modelMatrixUBO {};

    modelMatrixUBO.model = actors[actor].modelMatrix;
    modelMatrixUBO.lightSpaceMatrix = dirLightSpaceMatrix[currentLight];

    for (unsigned int j = 0; j < MAX_JOINTS_NUMBER; ++j) {
        modelMatrixUBO.jointMatrices[j] = actors[actor].jointMatrices[j];
    }

    void* modelMatrixData = nullptr;
    unsigned int shadowMapDirectionalLightDescriptorBindingIndex =
        descriptorSetsConfig[DescriptorSetDataLink::SHADOW_MAP_DIRECTIONAL_LIGHT]
            .descriptorsBindingsIDs[0];
    vkMapMemory(
        device,
        GPUDescriptors[descriptorBindingsConfig
                           [shadowMapDirectionalLightDescriptorBindingIndex]
                               .globalDescriptorOffset]
            .GPUBuffer->deviceMemory,
        currentImage * sizeof(modelMatrixUBO),
        sizeof(modelMatrixUBO),
        0,
        &modelMatrixData
    );
    memcpy(modelMatrixData, &modelMatrixUBO, sizeof(modelMatrixUBO));
    vkUnmapMemory(
        device,
        GPUDescriptors[descriptorBindingsConfig
                           [shadowMapDirectionalLightDescriptorBindingIndex]
                               .globalDescriptorOffset]
            .GPUBuffer->deviceMemory
    );
}

void CVulkanRenderer::updateSpotLightShadowMapMatrixUBO(
    uint32_t currentImage,
    uint32_t currentLight,
    unsigned int actor
) {
    ShadowMapMatrixUBO modelMatrixUBO {};

    modelMatrixUBO.model = actors[actor].modelMatrix;
    modelMatrixUBO.lightSpaceMatrix = spotLightSpaceMatrix[currentLight];

    for (unsigned int j = 0; j < MAX_JOINTS_NUMBER; ++j) {
        modelMatrixUBO.jointMatrices[j] = actors[actor].jointMatrices[j];
    }

    void* modelMatrixData;
    unsigned int shadowMapSpotLightDescriptorBindingIndex =
        descriptorSetsConfig[DescriptorSetDataLink::SHADOW_MAP_SPOT_LIGHT]
            .descriptorsBindingsIDs[0];
    vkMapMemory(
        device,
        GPUDescriptors
            [descriptorBindingsConfig[shadowMapSpotLightDescriptorBindingIndex]
                 .globalDescriptorOffset]
                .GPUBuffer->deviceMemory,
        currentImage * sizeof(modelMatrixUBO),
        sizeof(modelMatrixUBO),
        0,
        &modelMatrixData
    );
    memcpy(modelMatrixData, &modelMatrixUBO, sizeof(modelMatrixUBO));
    vkUnmapMemory(
        device,
        GPUDescriptors
            [descriptorBindingsConfig[shadowMapSpotLightDescriptorBindingIndex]
                 .globalDescriptorOffset]
                .GPUBuffer->deviceMemory
    );
}

void CVulkanRenderer::updatePointLightShadowMapMatrixUBO(
    [[maybe_unused]] uint32_t currentImage,
    uint32_t currentLight,
    uint32_t layer,
    unsigned int actor
) {
    PointLightShadowMapMatrixUBO modelMatrixUBO {};

    modelMatrixUBO.model = actors[actor].modelMatrix;

    modelMatrixUBO.lightSpaceMatrix =
        pointLights[currentLight].pointLightSpaceMatrix[layer];
    modelMatrixUBO.farPlane = 100.0f;
    modelMatrixUBO.lightPosition = pointLights[currentLight].position;

    for (unsigned int j = 0; j < MAX_JOINTS_NUMBER; ++j) {
        modelMatrixUBO.jointMatrices[j] = actors[actor].jointMatrices[j];
    }

    void* modelMatrixData;
    unsigned int shadowMapPointLightDescriptorBindingIndex =
        descriptorSetsConfig[DescriptorSetDataLink::SHADOW_MAP_POINT_LIGHT]
            .descriptorsBindingsIDs[0];
    vkMapMemory(
        device,
        GPUDescriptors
            [descriptorBindingsConfig[shadowMapPointLightDescriptorBindingIndex]
                 .globalDescriptorOffset]
                .GPUBuffer->deviceMemory,
        currentImage * sizeof(modelMatrixUBO),
        sizeof(modelMatrixUBO),
        0,
        &modelMatrixData
    );
    memcpy(modelMatrixData, &modelMatrixUBO, sizeof(modelMatrixUBO));
    vkUnmapMemory(
        device,
        GPUDescriptors
            [descriptorBindingsConfig[shadowMapPointLightDescriptorBindingIndex]
                 .globalDescriptorOffset]
                .GPUBuffer->deviceMemory
    );
}

void CVulkanRenderer::updateMatrixUniformBuffer(
    uint32_t offset,
    unsigned int actor
) {
    ModelMatrixUBO modelMatrixUBO {};

    modelMatrixUBO.model = actors[actor].modelMatrix;

    modelMatrixUBO.view = viewMatrix;
    modelMatrixUBO.proj = projectionMatrix;

    for (unsigned int j = 0; j < MAX_JOINTS_NUMBER; ++j) {
        modelMatrixUBO.jointMatrices[j] = actors[actor].jointMatrices[j];
    }

    modelMatrixUBO.ambient = actors[actor].ambient;
    modelMatrixUBO.shininess = actors[actor].shininess;

    for (uint32_t i = 0; i < directionalLightNumber; ++i) {
        modelMatrixUBO.dirSpaceMatrix[i] = dirLightSpaceMatrix[i];
    }

    for (uint32_t i = 0; i < spotLightNumber; ++i) {
        modelMatrixUBO.spotSpaceMatrix[i] = spotLightSpaceMatrix[i];
    }

    modelMatrixUBO.directionalLightsNumber = directionalLightNumber;
    modelMatrixUBO.spotLightsNumber = spotLightNumber;

    void* modelMatrixData;
    vkMapMemory(
        device,
        GPUDescriptors[DescriptorSetDataLink::MAIN_RENDER_MATRIX_UBO]
            .GPUBuffer->deviceMemory,
        sizeof(modelMatrixUBO) * offset,
        sizeof(modelMatrixUBO),
        0,
        &modelMatrixData
    );
    memcpy(modelMatrixData, &modelMatrixUBO, sizeof(modelMatrixUBO));
    vkUnmapMemory(
        device,
        GPUDescriptors[DescriptorSetDataLink::MAIN_RENDER_MATRIX_UBO]
            .GPUBuffer->deviceMemory
    );
}

void CVulkanRenderer::updateViewPositionUniformBuffer(
    uint32_t currentImage,
    uint32_t player
) {
    LightData lightDataUBO {};
    lightDataUBO.viewPosition = players[player].position;

    DirectionalLight directionalLight {};
    directionalLightNumber = directionalLights.size();
    assert(
        directionalLightNumber <= 4
        && "Directional lights number greater then 4"
    );
    for (unsigned int i = 0; i < directionalLightNumber; ++i) {
        RenderDirectionalLight dirLight = directionalLights[i];

        directionalLight.position = dirLight.position;
        directionalLight.direction = dirLight.direction;
        directionalLight.ambient = dirLight.ambient;
        directionalLight.diffuse = dirLight.diffuse;
        directionalLight.specular = dirLight.specular;

        lightDataUBO.directionalLights[i] = directionalLight;
    }
    lightDataUBO.directionalLightsArraySize = directionalLightNumber;

    pointLightNumber = pointLights.size();
    assert(
        pointLightNumber <= POINT_LIGHTS_NUMBER
        && "Point lights number greater than 32"
    );
    for (unsigned int i = 0; i < pointLightNumber; ++i) {
        RenderPointLight pointLight = pointLights[i];
        PointLight pointLightUBO {};

        pointLightUBO.position = pointLight.position;
        pointLightUBO.ambient = pointLight.ambient;
        pointLightUBO.diffuse = pointLight.diffuse;
        pointLightUBO.specular = pointLight.specular;
        pointLightUBO.constant = pointLight.constant;
        pointLightUBO.linear = pointLight.linear;
        pointLightUBO.quadratic = pointLight.quadratic;

        lightDataUBO.pointLights[i] = pointLightUBO;
    }
    lightDataUBO.pointLightsArraySize = pointLightNumber;
    lightDataUBO.farPlane = 100.0f;

    SpotLight spotLightUBO {};
    spotLightNumber = spotLights.size();
    assert(spotLightNumber <= 8 && "Spot light number greater then 8");
    for (unsigned int i = 0; i < spotLightNumber; ++i) {
        RenderSpotLight spotLight = spotLights[i];

        spotLightUBO.position = spotLight.position;
        spotLightUBO.direction = spotLight.direction;
        spotLightUBO.cutOff = std::cos(Radians(spotLight.cutOff));
        spotLightUBO.outerCutOff = std::cos(Radians(spotLight.outerCutOff));
        spotLightUBO.ambient = spotLight.ambient;
        spotLightUBO.diffuse = spotLight.diffuse;
        spotLightUBO.specular = spotLight.specular;
        spotLightUBO.constant = spotLight.constant;
        spotLightUBO.linear = spotLight.linear;
        spotLightUBO.quadratic = spotLight.quadratic;

        lightDataUBO.spotLights[i] = spotLightUBO;
    }
    lightDataUBO.spotLightArraySize = spotLightNumber;

    std::random_device rd;
    std::mt19937 mersenne(rd());
    std::uniform_int_distribution<int> distributionTileIndex(
        0,
        INDIRECT_TEXTURE_HEIGHT * INDIRECT_TEXTURE_WIDTH
    );

    if (print == true) {
        for (int i = 0;
             i < INDIRECT_TEXTURE_HEIGHT * INDIRECT_TEXTURE_WIDTH / 4 + 1;
             ++i) {
            for (int j = 0; j < 4; ++j) {
                int randomTileIndex = distributionTileIndex(mersenne);
                indirectTexture[i][j] = randomTileIndex;
            }
        }
    }
    print = false;
    lightDataUBO.tilesetTilesCount =
        Vector<float, 2>(TILESET_ROW, TILESET_COLUMN);
    lightDataUBO.tilesRaw = 8;
    lightDataUBO.tilesColumn = 8;
    lightDataUBO.debugShadowMode =
        imguiOverlay->showShadowMaps ? (imguiOverlay->shadowMapMode + 1) : 0;
    lightDataUBO.debugShadowLight = imguiOverlay->shadowMapLight;
    lightDataUBO.shadowsEnabled = imguiOverlay->shadowsEnabled ? 1 : 0;
    for (int i = 0;
         i < INDIRECT_TEXTURE_HEIGHT * INDIRECT_TEXTURE_WIDTH / 4 + 1;
         ++i) {
        lightDataUBO.indirectTexture[i] = indirectTexture[i];
    }

    void* data;
    unsigned int lightDataUboDescriptorBindingIndex =
        descriptorSetsConfig[DescriptorSetDataLink::MAIN_RENDER_LIGHT_DATA_UBO]
            .descriptorsBindingsIDs[0];
    vkMapMemory(
        device,
        GPUDescriptors[descriptorBindingsConfig[lightDataUboDescriptorBindingIndex]
                           .globalDescriptorOffset]
            .GPUBuffer->deviceMemory,
        sizeof(lightDataUBO) * currentImage,
        sizeof(lightDataUBO),
        0,
        &data
    );
    memcpy(data, &lightDataUBO, sizeof(lightDataUBO));
    vkUnmapMemory(
        device,
        GPUDescriptors[descriptorBindingsConfig[lightDataUboDescriptorBindingIndex]
                           .globalDescriptorOffset]
            .GPUBuffer->deviceMemory
    );
}

void CVulkanRenderer::mainRenderDrawFrame() {
    vkWaitForFences(
        device,
        1,
        &inFlightFences[currentFrame],
        VK_TRUE,
        UINT64_MAX
    );

    uint32_t imageIndex;
    // vkAcquireNextImageKHR give index of image that WILL BE SOON available for
    // rendering and signal imageAvailablesemaphore when its so. GraphicsQueue
    // waint for this semaphore bacause we pass it in submitInfo.
    VkResult result = vkAcquireNextImageKHR(
        device,
        swapChain,
        UINT64_MAX,
        imageAvailableSemaphores[currentFrame],
        VK_NULL_HANDLE,
        &imageIndex
    );

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        recreateSwapChain();
        return;
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("failed to acquire swap chain image!");
    }

    vkResetFences(device, 1, &inFlightFences[currentFrame]);
    vkResetCommandBuffer(
        mainRenderCommandBuffers[currentFrame],
        0 // VkCommandBufferResetFlagBits.
    );

    auto future1 = renderThreadPool->enqueue([this]() {
        directionalLightRecordCoomandBuffer(
            directionalLightSecondaryCommandBuffers,
            this->currentFrame
        );
    });

    auto future2 = renderThreadPool->enqueue([this]() {
        spotLightRecordCommandBuffer(
            spotLightSecondaryCommandBuffers,
            this->currentFrame
        );
    });

    auto future3 = renderThreadPool->enqueue([this]() {
        pointLightRecordCommandBuffer(
            pointLightSecondaryCommandBuffers,
            this->currentFrame
        );
    });

    future1.wait();
    future2.wait();
    future3.wait();

    VkCommandBufferBeginInfo beginInfo {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(mainRenderCommandBuffers[currentFrame], &beginInfo)
        != VK_SUCCESS) {
        throw std::runtime_error("failed to begin recording command buffer!");
    }
    for (uint32_t directionalLightCounter = 0;
         directionalLightCounter < directionalLights.size();
         ++directionalLightCounter) {
        VkExtent2D flatShadowMapExtent;
        flatShadowMapExtent.width = FLAT_SHADOW_MAP_SIZE;
        flatShadowMapExtent.height = FLAT_SHADOW_MAP_SIZE;
        executeSecondaryCommandBuffer(
            renderPasses[SpecificPipeline::DIRECTIONAL_LIGHT_PIPELINE],
            directionalLightShadowMapFrameBuffers[directionalLightCounter],
            flatShadowMapExtent,
            mainRenderCommandBuffers[currentFrame],
            directionalLightSecondaryCommandBuffers
                [currentFrame * directionalLightNumber + directionalLightCounter]
        );
    }
    for (uint32_t spotLightCounter = 0; spotLightCounter < spotLights.size();
         ++spotLightCounter) {
        VkExtent2D flatShadowMapExtent;
        flatShadowMapExtent.width = FLAT_SHADOW_MAP_SIZE;
        flatShadowMapExtent.height = FLAT_SHADOW_MAP_SIZE;
        executeSecondaryCommandBuffer(
            renderPasses[SpecificPipeline::SPOT_LIGHT_PIPELINE],
            spotLightShadowMapFrameBuffers[spotLightCounter],
            flatShadowMapExtent,
            mainRenderCommandBuffers[currentFrame],
            spotLightSecondaryCommandBuffers
                [currentFrame * spotLightNumber + spotLightCounter]
        );
    }
    for (uint32_t pointLightCounter = 0; pointLightCounter < pointLights.size();
         ++pointLightCounter) {
        uint32_t maxCubeMapLayers = 6;
        for (uint32_t cubeMapLayerCounter = 0;
             cubeMapLayerCounter < maxCubeMapLayers;
             ++cubeMapLayerCounter) {
            VkExtent2D extent;
            extent.width = SHADOW_MAP_SIZE;
            extent.height = SHADOW_MAP_SIZE;
            executeSecondaryCommandBuffer(
                renderPasses[SpecificPipeline::POINT_LIGHT_PIPELINE],
                pointLightShadowMapFrameBuffers[pointLightCounter]
                                               [cubeMapLayerCounter],
                extent,
                mainRenderCommandBuffers[currentFrame],
                pointLightSecondaryCommandBuffers
                    [currentFrame * pointLightNumber * maxCubeMapLayers
                     + pointLightCounter * maxCubeMapLayers
                     + cubeMapLayerCounter]
            );
        }
    }

    recordCommandBuffer(mainRenderCommandBuffers[currentFrame], imageIndex);
    hudRecordCommandBuffer(mainRenderCommandBuffers[currentFrame], imageIndex);
    fontRecordCommandBuffer(mainRenderCommandBuffers[currentFrame], imageIndex);
    if (isInventoryOpened) {
        uiRecordCommandBuffer(
            mainRenderCommandBuffers[currentFrame],
            imageIndex
        );
        uiIconsRecordCommandBuffer(
            mainRenderCommandBuffers[currentFrame],
            imageIndex
        );
    }

    imguiOverlay->recordCommandBuffer(
        mainRenderCommandBuffers[currentFrame],
        imageIndex
    );
    hudScreenRecordCommandBuffer(
        mainRenderCommandBuffers[currentFrame],
        imageIndex
    );

    VkSubmitInfo submitInfo {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    // GraphicsQueue wait for swapchain image when its become available.
    VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
    VkPipelineStageFlags waitStages[] = {
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
    };
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &mainRenderCommandBuffers[currentFrame];

    VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[imageIndex]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame])
        != VK_SUCCESS) {
        throw std::runtime_error("failed to submit draw command buffer!");
    }

    VkPresentInfoKHR presentInfo {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = {swapChain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;

    presentInfo.pImageIndices = &imageIndex;

    result = vkQueuePresentKHR(presentQueue, &presentInfo);

    if (frameCounter == 10) {
        vkDeviceWaitIdle(device);
    }

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR
        || framebufferResized) {
        framebufferResized = false;
        recreateSwapChain();
    } else if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to present swap chain image!");
    }

    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void CVulkanRenderer::directionalLightShadowMapDrawFrame() {
    vkWaitForFences(
        device,
        1,
        &directionalLightShadowMapInFlightFences[directionalLightCurrentFrame],
        VK_TRUE,
        UINT64_MAX
    );

    [[maybe_unused]] uint32_t imageIndex = 0;

    vkResetFences(
        device,
        1,
        &directionalLightShadowMapInFlightFences[directionalLightCurrentFrame]
    );
    vkResetCommandBuffer(
        directionalLightCommandBuffers[directionalLightCurrentFrame],
        0 // VkCommandBufferResetFlagBits.
    );
    VkSubmitInfo submitInfo {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers =
        &directionalLightCommandBuffers[directionalLightCurrentFrame];

    if (vkQueueSubmit(
            graphicsQueue,
            1,
            &submitInfo,
            directionalLightShadowMapInFlightFences[directionalLightCurrentFrame]
        )
        != VK_SUCCESS) {
        throw std::runtime_error("failed to submit draw command buffer!");
    }

    directionalLightCurrentFrame =
        (directionalLightCurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void CVulkanRenderer::spotLightShadowMapDrawFrame() {
    vkWaitForFences(
        device,
        1,
        &spotLightShadowMapInFlightFences[spotLightCurrentFrame],
        VK_TRUE,
        UINT64_MAX
    );

    [[maybe_unused]] uint32_t imageIndex = 0;

    vkResetFences(
        device,
        1,
        &spotLightShadowMapInFlightFences[spotLightCurrentFrame]
    );
    vkResetCommandBuffer(
        spotLightCommandBuffers[spotLightCurrentFrame],
        0 // VkCommandBufferResetFlagBits.
    );

    VkSubmitInfo submitInfo {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers =
        &spotLightCommandBuffers[spotLightCurrentFrame];

    if (vkQueueSubmit(
            graphicsQueue,
            1,
            &submitInfo,
            spotLightShadowMapInFlightFences[spotLightCurrentFrame]
        )
        != VK_SUCCESS) {
        throw std::runtime_error("failed to submit draw command buffer!");
    }

    spotLightCurrentFrame = (spotLightCurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void CVulkanRenderer::pointLightShadowMapDrawFrame() {
    vkWaitForFences(
        device,
        1,
        &pointLightShadowMapInFlightFences[pointLightCurrentFrame],
        VK_TRUE,
        UINT64_MAX
    );

    [[maybe_unused]] uint32_t imageIndex = 0;

    vkResetFences(
        device,
        1,
        &pointLightShadowMapInFlightFences[pointLightCurrentFrame]
    );
    vkResetCommandBuffer(
        pointLightCommandBuffers[pointLightCurrentFrame],
        0 // VkCommandBufferResetFlagBits.
    );

    VkSubmitInfo submitInfo {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers =
        &pointLightCommandBuffers[pointLightCurrentFrame];

    if (vkQueueSubmit(
            graphicsQueue,
            1,
            &submitInfo,
            pointLightShadowMapInFlightFences[pointLightCurrentFrame]
        )
        != VK_SUCCESS) {
        throw std::runtime_error("failed to submit draw command buffer!");
    }

    pointLightCurrentFrame =
        (pointLightCurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void CVulkanRenderer::directionalLightRecordCoomandBuffer(
    std::vector<VkCommandBuffer>& commandBuffers,
    [[maybe_unused]] uint32_t currentFrame
) {
    for (uint32_t directionalLightCounter = 0;
         directionalLightCounter < directionalLights.size();
         ++directionalLightCounter) {
        VkCommandBufferInheritanceInfo inheritanceInfo {};
        inheritanceInfo.sType =
            VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
        inheritanceInfo.renderPass =
            renderPasses[SpecificPipeline::DIRECTIONAL_LIGHT_PIPELINE];
        inheritanceInfo.framebuffer =
            directionalLightShadowMapFrameBuffers[directionalLightCounter];
        inheritanceInfo.subpass = 0;

        VkCommandBufferBeginInfo beginInfo {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
        beginInfo.pInheritanceInfo = &inheritanceInfo;

        VkCommandBuffer commandBuffer = commandBuffers
            [currentFrame * directionalLightNumber + directionalLightCounter];
        if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error(
                "failed to begin recording command buffer!"
            );
        }

        VkViewport shadowMapViewPort;
        shadowMapViewPort.height = FLAT_SHADOW_MAP_SIZE;
        shadowMapViewPort.width = FLAT_SHADOW_MAP_SIZE;
        shadowMapViewPort.minDepth = 0.0f;
        shadowMapViewPort.maxDepth = 1.0f;
        shadowMapViewPort.x = 0;
        shadowMapViewPort.y = 0;
        vkCmdSetViewport(commandBuffer, 0, 1, &shadowMapViewPort);

        VkRect2D shadowMapScissor;
        shadowMapScissor.extent.width = FLAT_SHADOW_MAP_SIZE;
        shadowMapScissor.extent.height = FLAT_SHADOW_MAP_SIZE;
        shadowMapScissor.offset.x = 0;
        shadowMapScissor.offset.y = 0;
        vkCmdSetScissor(commandBuffer, 0, 1, &shadowMapScissor);

        vkCmdBindPipeline(
            commandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            pipelineConfigs[SpecificPipeline::DIRECTIONAL_LIGHT_PIPELINE].pipeline
        );
        dirLightSpaceMatrix[directionalLightCounter] =
            directionalLights[directionalLightCounter]
                .DirectionalLightSpaceMatrix;

        uint32_t actorsNumber = actors.size();
        for (unsigned int actorCounter = 0; actorCounter < actorsNumber;
             ++actorCounter) {
            RenderActor actor = actors[actorCounter];
            unsigned int meshId = actor.meshID;
            unsigned int uboDirectionalLightIndex = directionalLightNumber
                    * actorsNumber * directionalLightCurrentFrame
                + actorsNumber * directionalLightCounter + actorCounter;

            updateDirectionalLightShadowMapMatrixUBO(
                uboDirectionalLightIndex,
                directionalLightCounter,
                actorCounter
            );
            const unsigned int linkedDescriptorSetID =
                pipelineConfigs[SpecificPipeline::DIRECTIONAL_LIGHT_PIPELINE]
                    .linkedDescriptorSetIDs[0];
            const DescriptorSet& currentDescriptorSet =
                descriptorSetsConfig[linkedDescriptorSetID];
            vkCmdBindDescriptorSets(
                commandBuffer,
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                pipelineConfigs[SpecificPipeline::DIRECTIONAL_LIGHT_PIPELINE]
                    .pipelineLayout,
                0,
                1,
                &(*(descriptorSetsChunks.data()
                    + currentDescriptorSet.descriptorSetOffset
                    + uboDirectionalLightIndex)),
                0,
                nullptr
            );

            VkBuffer vertexBuffers[] = {vertexBufferContainer[meshId]};
            VkDeviceSize offsets[] = {0};
            vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

            vkCmdBindIndexBuffer(
                commandBuffer,
                indexBufferContainer[meshId],
                0,
                VK_INDEX_TYPE_UINT32
            );

            unsigned int indicesContainerSize = aIndices_[meshId].size();
            vkCmdDrawIndexed(
                commandBuffer,
                static_cast<uint32_t>(indicesContainerSize),
                1,
                0,
                0,
                0
            );
        }

        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to record command buffer!");
        }
    }
}

void CVulkanRenderer::spotLightRecordCommandBuffer(
    std::vector<VkCommandBuffer>& commandBuffers,
    [[maybe_unused]] uint32_t currentFrame
) {
    for (uint32_t spotLightCounter = 0; spotLightCounter < spotLights.size();
         ++spotLightCounter) {
        VkCommandBufferInheritanceInfo inheritanceInfo {};
        inheritanceInfo.sType =
            VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
        inheritanceInfo.renderPass =
            renderPasses[SpecificPipeline::SPOT_LIGHT_PIPELINE];
        inheritanceInfo.framebuffer =
            spotLightShadowMapFrameBuffers[spotLightCounter];
        inheritanceInfo.subpass = 0;

        VkCommandBufferBeginInfo beginInfo {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
        beginInfo.pInheritanceInfo = &inheritanceInfo;

        VkCommandBuffer commandBuffer =
            commandBuffers[currentFrame * spotLightNumber + spotLightCounter];
        if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error(
                "failed to begin recording command buffer!"
            );
        }

        VkViewport spotLightShadowMapViewPort;
        spotLightShadowMapViewPort.height = FLAT_SHADOW_MAP_SIZE;
        spotLightShadowMapViewPort.width = FLAT_SHADOW_MAP_SIZE;
        spotLightShadowMapViewPort.minDepth = 0.0f;
        spotLightShadowMapViewPort.maxDepth = 1.0f;
        spotLightShadowMapViewPort.x = 0;
        spotLightShadowMapViewPort.y = 0;
        vkCmdSetViewport(commandBuffer, 0, 1, &spotLightShadowMapViewPort);

        VkRect2D spotLightShadowMapScissor;
        spotLightShadowMapScissor.extent.width = FLAT_SHADOW_MAP_SIZE;
        spotLightShadowMapScissor.extent.height = FLAT_SHADOW_MAP_SIZE;
        spotLightShadowMapScissor.offset.x = 0;
        spotLightShadowMapScissor.offset.y = 0;
        vkCmdSetScissor(commandBuffer, 0, 1, &spotLightShadowMapScissor);

        vkCmdBindPipeline(
            commandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            pipelineConfigs[SpecificPipeline::SPOT_LIGHT_PIPELINE].pipeline
        );

        spotLightSpaceMatrix[spotLightCounter] =
            spotLights[spotLightCounter].SpotLigthSpaceMatrix;
        uint32_t actorsNumber = actors.size();
        for (unsigned int actorsCounter = 0; actorsCounter < actorsNumber;
             ++actorsCounter) {
            RenderActor actor = actors[actorsCounter];
            unsigned int meshID = actor.meshID;
            unsigned int uboSpotLightIndex =
                spotLightNumber * actorsNumber * spotLightCurrentFrame
                + actorsNumber * spotLightCounter + actorsCounter;

            updateSpotLightShadowMapMatrixUBO(
                uboSpotLightIndex,
                spotLightCounter,
                actorsCounter
            );
            const unsigned int linkedDescriptorSetID =
                pipelineConfigs[SpecificPipeline::SPOT_LIGHT_PIPELINE]
                    .linkedDescriptorSetIDs[0];
            const DescriptorSet& currentDescriptorSet =
                descriptorSetsConfig[linkedDescriptorSetID];
            vkCmdBindDescriptorSets(
                commandBuffer,
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                pipelineConfigs[SpecificPipeline::SPOT_LIGHT_PIPELINE]
                    .pipelineLayout,
                0,
                1,
                &(*(descriptorSetsChunks.data()
                    + currentDescriptorSet.descriptorSetOffset
                    + uboSpotLightIndex)),
                0,
                nullptr
            );
            VkBuffer vertexBuffers[] = {vertexBufferContainer[meshID]};
            VkDeviceSize offsets[] = {0};
            vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

            vkCmdBindIndexBuffer(
                commandBuffer,
                indexBufferContainer[meshID],
                0,
                VK_INDEX_TYPE_UINT32
            );

            unsigned int indicesContainerSize = aIndices_[meshID].size();
            vkCmdDrawIndexed(
                commandBuffer,
                static_cast<uint32_t>(indicesContainerSize),
                1,
                0,
                0,
                0
            );
        }

        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to record command buffer!");
        }
    }
}

void CVulkanRenderer::pointLightRecordCommandBuffer(
    std::vector<VkCommandBuffer>& commandBuffers,
    [[maybe_unused]] uint32_t currentFrame
) {
    for (uint32_t pointLightCounter = 0; pointLightCounter < pointLights.size();
         ++pointLightCounter) {
        uint32_t maxCubeMapLayers = 6;
        // 6 is a number of cube map layers.
        for (uint32_t cubeMapLayerCounter = 0;
             cubeMapLayerCounter < maxCubeMapLayers;
             ++cubeMapLayerCounter) {
            VkCommandBufferInheritanceInfo inheritanceInfo {};
            inheritanceInfo.sType =
                VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
            inheritanceInfo.renderPass =
                renderPasses[SpecificPipeline::POINT_LIGHT_PIPELINE];
            inheritanceInfo.framebuffer =
                pointLightShadowMapFrameBuffers[pointLightCounter]
                                               [cubeMapLayerCounter];
            inheritanceInfo.subpass = 0;

            VkCommandBufferBeginInfo beginInfo {};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            beginInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
            beginInfo.pInheritanceInfo = &inheritanceInfo;

            VkCommandBuffer commandBuffer = commandBuffers
                [currentFrame * pointLightNumber * maxCubeMapLayers
                 + pointLightCounter * maxCubeMapLayers + cubeMapLayerCounter];
            if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
                throw std::runtime_error(
                    "failed to begin recording command buffer!"
                );
            }

            VkViewport pointLightShadowMapViewPort;
            pointLightShadowMapViewPort.height = SHADOW_MAP_SIZE;
            pointLightShadowMapViewPort.width = SHADOW_MAP_SIZE;
            pointLightShadowMapViewPort.minDepth = 0.0f;
            pointLightShadowMapViewPort.maxDepth = 1.0f;
            pointLightShadowMapViewPort.x = 0;
            pointLightShadowMapViewPort.y = 0;
            vkCmdSetViewport(commandBuffer, 0, 1, &pointLightShadowMapViewPort);

            VkRect2D pointLightShadowMapScissor;
            pointLightShadowMapScissor.extent.width = SHADOW_MAP_SIZE;
            pointLightShadowMapScissor.extent.height = SHADOW_MAP_SIZE;
            pointLightShadowMapScissor.offset.x = 0;
            pointLightShadowMapScissor.offset.y = 0;
            vkCmdSetScissor(commandBuffer, 0, 1, &pointLightShadowMapScissor);

            vkCmdBindPipeline(
                commandBuffer,
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                pipelineConfigs[SpecificPipeline::POINT_LIGHT_PIPELINE].pipeline
            );

            uint32_t actorsNumber = actors.size();
            for (unsigned int actorCounter = 0; actorCounter < actorsNumber;
                 ++actorCounter) {
                RenderActor actor = actors[actorCounter];
                unsigned int meshID = actor.meshID;

                unsigned int uboIndex = pointLightNumber * actorsNumber
                        * maxCubeMapLayers * pointLightCurrentFrame
                    + actorsNumber * maxCubeMapLayers * pointLightCounter
                    + maxCubeMapLayers * actorCounter + cubeMapLayerCounter;

                updatePointLightShadowMapMatrixUBO(
                    uboIndex,
                    pointLightCounter,
                    cubeMapLayerCounter,
                    actorCounter
                );
                const unsigned int linkedDescriptorSetID =
                    pipelineConfigs[SpecificPipeline::POINT_LIGHT_PIPELINE]
                        .linkedDescriptorSetIDs[0];
                const DescriptorSet& currentDescriptorSet =
                    descriptorSetsConfig[linkedDescriptorSetID];
                vkCmdBindDescriptorSets(
                    commandBuffer,
                    VK_PIPELINE_BIND_POINT_GRAPHICS,
                    pipelineConfigs[SpecificPipeline::POINT_LIGHT_PIPELINE]
                        .pipelineLayout,
                    0,
                    1,
                    &(*(descriptorSetsChunks.data()
                        + currentDescriptorSet.descriptorSetOffset + uboIndex)),
                    0,
                    nullptr
                );

                VkBuffer vertexBuffers[] = {vertexBufferContainer[meshID]};
                VkDeviceSize offsets[] = {0};
                vkCmdBindVertexBuffers(
                    commandBuffer,
                    0,
                    1,
                    vertexBuffers,
                    offsets
                );

                vkCmdBindIndexBuffer(
                    commandBuffer,
                    indexBufferContainer[meshID],
                    0,
                    VK_INDEX_TYPE_UINT32
                );

                unsigned int indicesContainerSize = aIndices_[meshID].size();
                vkCmdDrawIndexed(
                    commandBuffer,
                    static_cast<uint32_t>(indicesContainerSize),
                    1,
                    0,
                    0,
                    0
                );
            }

            if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
                throw std::runtime_error("failed to record command buffer!");
            }
        }
    }
}

VkShaderModule CVulkanRenderer::createShaderModule(
    const std::vector<char>& code
) {
    VkShaderModuleCreateInfo createInfo {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule)
        != VK_SUCCESS) {
        throw std::runtime_error("failed to create shader module!");
    }

    return shaderModule;
}

VkSurfaceFormatKHR CVulkanRenderer::chooseSwapSurfaceFormat(
    const std::vector<VkSurfaceFormatKHR>& availableFormats
) {
    for (const auto& availableFormat : availableFormats) {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB
            && availableFormat.colorSpace
                == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormat;
        }
    }

    return availableFormats[0];
}

VkPresentModeKHR CVulkanRenderer::chooseSwapPresentMode(
    const std::vector<VkPresentModeKHR>& availablePresentModes
) {
    for (const auto& availablePresentMode : availablePresentModes) {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR
            || availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
            return availablePresentMode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D CVulkanRenderer::chooseSwapExtent(
    const VkSurfaceCapabilitiesKHR& capabilities
) {
    if (capabilities.currentExtent.width
        != std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    } else {
        VkExtent2D actualExtent {
            .width = Window->width,
            .height = Window->height
        };
        actualExtent.width = std::clamp(
            actualExtent.width,
            capabilities.minImageExtent.width,
            capabilities.maxImageExtent.width
        );
        actualExtent.height = std::clamp(
            actualExtent.height,
            capabilities.minImageExtent.height,
            capabilities.maxImageExtent.height
        );

        return actualExtent;
    }
}

SwapChainSupportDetails CVulkanRenderer::querySwapChainSupport(
    VkPhysicalDevice device
) {
    SwapChainSupportDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
        device,
        surface,
        &details.capabilities
    );

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

    if (formatCount != 0) {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(
            device,
            surface,
            &formatCount,
            details.formats.data()
        );
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(
        device,
        surface,
        &presentModeCount,
        nullptr
    );

    if (presentModeCount != 0) {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(
            device,
            surface,
            &presentModeCount,
            details.presentModes.data()
        );
    }

    return details;
}

bool CVulkanRenderer::isDeviceSuitable(VkPhysicalDevice device) {
    QueueFamilyIndices indices = findQueueFamilies(device);

    bool extensionsSupported = checkDeviceExtensionSupport(device);

    bool swapChainAdequate = false;
    if (extensionsSupported) {
        SwapChainSupportDetails swapChainSupport =
            querySwapChainSupport(device);
        swapChainAdequate = !swapChainSupport.formats.empty()
            && !swapChainSupport.presentModes.empty();
    }

    VkPhysicalDeviceFeatures supportedFeatures;
    vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

    return indices.isComplete() && extensionsSupported && swapChainAdequate
        && supportedFeatures.samplerAnisotropy;
}

bool CVulkanRenderer::checkDeviceExtensionSupport(VkPhysicalDevice device) {
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(
        device,
        nullptr,
        &extensionCount,
        nullptr
    );

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(
        device,
        nullptr,
        &extensionCount,
        availableExtensions.data()
    );

    std::set<std::string> requiredExtensions(
        deviceExtensions.begin(),
        deviceExtensions.end()
    );

    for (const auto& extension : availableExtensions) {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

QueueFamilyIndices CVulkanRenderer::findQueueFamilies(VkPhysicalDevice device) {
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(
        device,
        &queueFamilyCount,
        queueFamilies.data()
    );

    int i = 0;
    for (const auto& queueFamily : queueFamilies) {
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphics_family = i;
        }

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(
            device,
            i,
            surface,
            &presentSupport
        );

        if (presentSupport) {
            indices.present_family = i;
        }

        if (indices.isComplete()) {
            break;
        }

        i++;
    }

    return indices;
}

std::vector<const char*> CVulkanRenderer::getRequiredExtensions() {
#ifdef VK_USE_PLATFORM_XLIB_KHR
    std::vector<const char*> pRequiredExtentions = {
        "VK_KHR_xlib_surface",
        "VK_EXT_acquire_xlib_display",
        "VK_KHR_display",
        "VK_KHR_surface",
        "VK_EXT_direct_mode_display"
    };
#endif

#ifdef VK_USE_PLATFORM_XCB_KHR
    std::vector<const char*> pRequiredExtentions = {
        "VK_KHR_xcb_surface",
        "VK_KHR_display",
        "VK_KHR_surface",
        "VK_EXT_direct_mode_display"
    };
#endif

#ifdef VK_USE_PLATFORM_WIN32_KHR
    std::vector<const char*> pRequiredExtentions = {
        "VK_KHR_win32_surface",
        "VK_KHR_surface"
    };
#endif

#ifdef VK_USE_PLATFORM_WAYLAND_KHR
    std::vector<const char*> pRequiredExtentions = {
        "VK_KHR_wayland_surface",
        "VK_KHR_display",
        "VK_EXT_direct_mode_display",
        "VK_KHR_surface"
    };
#endif

    if (enableValidationLayers) {
        pRequiredExtentions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return pRequiredExtentions;
}

bool CVulkanRenderer::checkValidationLayerSupport() {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char* layerName : validationLayers) {
        bool layerFound = false;

        for (const auto& layerProperties : availableLayers) {
            if (strcmp(layerName, layerProperties.layerName) == 0) {
                layerFound = true;
                break;
            }
        }

        if (!layerFound) {
            return false;
        }
    }

    return true;
}

VkDescriptorBufferInfo CVulkanRenderer::createDescriptorBufferInfo(
    VkBuffer ubo,
    const VkDeviceSize& uboStructSize,
    const VkDeviceSize& offsetStep
) {
    VkDescriptorBufferInfo uboBufferInfo {};
    uboBufferInfo.buffer = ubo;
    uboBufferInfo.offset = offsetStep * uboStructSize;
    uboBufferInfo.range = uboStructSize;

    return uboBufferInfo;
}

VkDescriptorImageInfo CVulkanRenderer::createDescriptorImageInfo(
    const VK_Image& textureImage,
    VkImageLayout layout,
    unsigned int textureViewIndex,
    VkSampler textureSampler
) {
    VkDescriptorImageInfo imageInfo {};
    imageInfo.imageLayout = layout;
    imageInfo.imageView = textureImage.views[textureViewIndex];
    imageInfo.sampler = textureSampler;

    return imageInfo;
}

std::vector<char> CVulkanRenderer::readFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        throw std::runtime_error("failed to open file!");
    }

    size_t fileSize = (size_t)file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();

    return buffer;
}

VKAPI_ATTR VkBool32 VKAPI_CALL CVulkanRenderer::debugCallback(
    [[maybe_unused]] VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    [[maybe_unused]] VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    [[maybe_unused]] void* pUserData
) {
    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

    return VK_FALSE;
}
} // namespace glvm

namespace glvm {

void CJsonParser::ReadFile(const char* _filePath) {
    const char* _pJsonFilePath = _filePath;
    std::ifstream jsonFileInputStream;
    std::stringstream jsonFileOutputStream;

    jsonFileInputStream.open(_pJsonFilePath);
    if (jsonFileInputStream.good()) {
        jsonFileOutputStream << jsonFileInputStream.rdbuf();
        jsonFileInputStream.close();
        sJsonFileData_ = jsonFileOutputStream.str();
    } else {
        std::cout << "Error of reading json file" << std::endl;
        return;
    }

    pJsonFileData_ = sJsonFileData_.c_str();
}

void CJsonParser::Parse() {
    currentChar_ = pJsonFileData_[globalFileCounter_];

    while (currentChar_ != '\0') {
        currentChar_ = pJsonFileData_[globalFileCounter_];

        if (currentChar_ == '"' && keyFlag) {
            lastKey_ = StringParse();
            while (currentChar_ == ' ' || currentChar_ == ':') {
                ++globalFileCounter_;
                currentChar_ = pJsonFileData_[globalFileCounter_];
            }
        }

        if (currentChar_ == '"') {
            bufferString_ = StringParse();

            if (keyFlag) {
                JsonValue jsonString(bufferString_);
                (*stackOfJsonValues_.back()->value.object)[lastKey_.c_str()] =
                    jsonString;
            } else {
                JsonValue jsonString(bufferString_);
                stackOfJsonValues_.back()->value.array->push_back(jsonString);
            }
        } else if (
            (currentChar_ >= '0' && currentChar_ <= '9') || currentChar_ == '+'
            || currentChar_ == '-'
        ) {
            bufferString_ = NumberAsStringParse();
            std::vector<char> vector = StringToVectorOfChars(bufferString_);
            double fNumber = 0.0f;
            int iNumber = 0;
            if (IsContainChar(bufferString_, '.')) {
                fNumber = ParseFloating(vector);

                if (keyFlag) {
                    JsonValue jsonFloat(fNumber);
                    (*stackOfJsonValues_.back()
                          ->value.object)[lastKey_.c_str()] = jsonFloat;
                } else {
                    JsonValue jsonFloat(fNumber);
                    stackOfJsonValues_.back()->value.array->push_back(jsonFloat);
                }
            } else {
                iNumber = ParseInteger(vector);

                if (keyFlag) {
                    JsonValue jsonInt(iNumber);
                    (*stackOfJsonValues_.back()
                          ->value.object)[lastKey_.c_str()] = jsonInt;
                } else {
                    JsonValue jsonInt(iNumber);
                    stackOfJsonValues_.back()->value.array->push_back(jsonInt);
                }
            }

        } else if (
            currentChar_ == 't' || currentChar_ == 'f' || currentChar_ == 'n'
        ) {
            std::string boolOrNullString = BoolOrNullParse();

            if (boolOrNullString == "true") {
                if (keyFlag) {
                    JsonValue jsonTrue(true);
                    (*stackOfJsonValues_.back()
                          ->value.object)[lastKey_.c_str()] = jsonTrue;
                } else {
                    JsonValue jsonTrue(true);
                    stackOfJsonValues_.back()->value.array->push_back(jsonTrue);
                }
            } else if (boolOrNullString == "false") {
                if (keyFlag) {
                    JsonValue jsonFalse(false);
                    (*stackOfJsonValues_.back()
                          ->value.object)[lastKey_.c_str()] = jsonFalse;
                } else {
                    JsonValue jsonFalse(false);
                    stackOfJsonValues_.back()->value.array->push_back(jsonFalse);
                }
            } else if (boolOrNullString == "null") {
                if (keyFlag) {
                    JsonValue jsonNull;
                    jsonNull.type = JsonNull;
                    jsonNull.value.null = NULL;
                    (*stackOfJsonValues_.back()
                          ->value.object)[lastKey_.c_str()] = jsonNull;
                } else {
                    JsonValue jsonNull;
                    jsonNull.type = JsonNull;
                    jsonNull.value.null = NULL;
                    stackOfJsonValues_.back()->value.array->push_back(jsonNull);
                }
            }
        } else if (currentChar_ == '{') {
            if (stackOfJsonValues_.size() == 0) {
                root_ = new JsonValue;
                *root_ = CreateJsonHashMap();
                stackOfJsonValues_.push_back(root_);
            } else if (keyFlag) {
                JsonValue jsonObject = CreateJsonHashMap();
                (*stackOfJsonValues_.back()->value.object)[lastKey_.c_str()] =
                    jsonObject;
                stackOfJsonValues_.push_back(&(
                    *stackOfJsonValues_.back()->value.object
                )[lastKey_.c_str()]);
            } else if (!keyFlag) {
                JsonValue jsonObject = CreateJsonHashMap();
                stackOfJsonValues_.back()->value.array->push_back(jsonObject);
                stackOfJsonValues_.push_back(
                    &stackOfJsonValues_.back()->value.array->back()
                );
            }

            keyFlag = true;
        } else if (currentChar_ == '[') {
            if (stackOfJsonValues_.size() == 0) {
                root_ = new JsonValue;
                *root_ = CreateJsonArray();
                stackOfJsonValues_.push_back(root_);
            } else if (keyFlag) {
                JsonValue jsonArray = CreateJsonArray();
                (*stackOfJsonValues_.back()->value.object)[lastKey_.c_str()] =
                    jsonArray;
                stackOfJsonValues_.push_back(&(
                    *stackOfJsonValues_.back()->value.object
                )[lastKey_.c_str()]);
            } else if (!keyFlag) {
                JsonValue jsonArray = CreateJsonArray();
                stackOfJsonValues_.back()->value.array->push_back(jsonArray);
                stackOfJsonValues_.push_back(
                    &stackOfJsonValues_.back()->value.array->back()
                );
            }

            keyFlag = false;
        } else if (currentChar_ == '}') {
            stackOfJsonValues_.pop_back();
            if (stackOfJsonValues_.size()
                && stackOfJsonValues_.back()->type == JsonObject) {
                keyFlag = true;
            } else {
                keyFlag = false;
            }

        } else if (currentChar_ == ']') {
            stackOfJsonValues_.pop_back();
            if (stackOfJsonValues_.size()
                && stackOfJsonValues_.back()->type == JsonObject) {
                keyFlag = true;
            } else {
                keyFlag = false;
            }
        }

        ++globalFileCounter_;
    }
}

JsonValue CJsonParser::CreateJsonHashMap() {
    JsonValue jsonObject;
    jsonObject.type = JsonObject;
    jsonObject.value.object = new HashMap<JsonValue>;
    return jsonObject;
}

JsonValue CJsonParser::CreateJsonArray() {
    JsonValue jsonArray;
    jsonArray.type = JsonArray;
    jsonArray.value.array = new std::vector<JsonValue>;
    return jsonArray;
}

std::string CJsonParser::BoolOrNullParse() {
    std::string boolOrNullString = "";
    while (1) {
        currentChar_ = pJsonFileData_[globalFileCounter_];
        if (currentChar_ >= 'a' && currentChar_ <= 'z') {
            boolOrNullString.push_back(currentChar_);
            ++globalFileCounter_;
        } else {
            return boolOrNullString;
        }
    }
}

bool CJsonParser::IsContainChar(std::string _string, char _char) {
    for (unsigned int i = 0; i < _string.size(); ++i) {
        if (_string[i] == _char) {
            return true;
        }
    }

    return false;
}

std::string CJsonParser::NumberAsStringParse() {
    std::string numberAsString = "";
    while (1) {
        currentChar_ = pJsonFileData_[globalFileCounter_];
        if ((currentChar_ >= '0' && currentChar_ <= '9') || currentChar_ == '+'
            || currentChar_ == '-' || currentChar_ == 'e') {
            numberAsString.push_back(currentChar_);
            ++globalFileCounter_;
        } else if (currentChar_ == '.') {
            numberAsString.push_back(currentChar_);
            ++globalFileCounter_;
        } else {
            return numberAsString;
        }
    }
}

std::string CJsonParser::StringParse() {
    ++globalFileCounter_;
    std::string localBuffer = "";
    while (1) {
        currentChar_ = pJsonFileData_[globalFileCounter_];
        if (currentChar_ == '"') {
            ++globalFileCounter_;
            currentChar_ = pJsonFileData_[globalFileCounter_];
            return localBuffer;
        } else {
            localBuffer.push_back(currentChar_);
            ++globalFileCounter_;
        }
    }
}

std::vector<char> CJsonParser::StringToVectorOfChars(std::string _string) {
    std::vector<char> vectorWithChars;
    for (unsigned int i = 0; i < _string.size(); ++i) {
        vectorWithChars.push_back(_string[i]);
    }

    return vectorWithChars;
}

int CJsonParser::ParseInteger(std::vector<char> _word) {
    std::vector<int> baseContainer;

    for (unsigned int i = 0; i < _word.size(); ++i) {
        baseContainer.push_back(_word[i] - 48);
    }

    int iResult = 0;
    bool negateFlag = false;

    unsigned int baseContainerSize = baseContainer.size();
    for (unsigned int i = 0; i < baseContainerSize; ++i) {
        if (baseContainer[i] == -3 && i == 0) {
            negateFlag = true;
            continue;
        } else if (baseContainer[i] == -5 && i == 0) {
            continue;
        }

        iResult += baseContainer[i] * std::pow(10, (baseContainerSize - 1) - i);
    }

    if (negateFlag) {
        iResult *= -1;
    }

    return iResult;
}

double CJsonParser::ParseFloating(std::vector<char> _word) {
    std::vector<int> baseContainer;

    for (unsigned int i = 0; i < _word.size(); ++i) {
        baseContainer.push_back(_word[i] - 48);
    }

    int integerPart = 0;
    double floatingPart = 0;
    int eNumber = 0;
    std::vector<int> integerPartContainer;
    std::vector<int> floatingPartContainer;
    std::vector<int> ePartContainer;
    bool dotFlag = false;
    bool negateFlag = false;
    bool eFlag = false;
    // False value equal "+" sign.
    bool eSign = false;
    unsigned int baseContainerSize = baseContainer.size();

    if (baseContainer[0] == -3) {
        negateFlag = true;
    }

    for (unsigned int i = 0; i < baseContainerSize; ++i) {
        if (negateFlag && i == 0) {
            continue;
        } else if (baseContainer[i] == -5 && i == 0) {
            continue;
        } else if (baseContainer[i] == -2) {
            dotFlag = true;
            continue;
        } else if (baseContainer[i] == 53) {
            eFlag = true;
            continue;
        }

        if (eFlag) {
            if (baseContainer[i] == -5) {
                continue;
            } else if (baseContainer[i] == -3) {
                eSign = true;
                continue;
            }

            ePartContainer.push_back(baseContainer[i]);
            continue;
        }

        if (baseContainer[i] >= 0 && baseContainer[i] <= 9) {
            if (dotFlag) {
                floatingPartContainer.push_back(baseContainer[i]);
            } else {
                integerPartContainer.push_back(baseContainer[i]);
            }
        } else {
            std::cout << "Element is not a number" << std::endl;
            return NAN;
        }
    }

    unsigned int ePartContainerSize = ePartContainer.size();
    for (unsigned int i = 0; i < ePartContainerSize; ++i) {
        eNumber +=
            ePartContainer[i] * std::pow(10, (ePartContainerSize - 1) - i);
    }

    unsigned int integerPartContainerSize = integerPartContainer.size();
    for (unsigned int i = 0; i < integerPartContainerSize; ++i) {
        integerPart += integerPartContainer[i]
            * std::pow(10, (integerPartContainerSize - 1) - i);
    }

    unsigned int floatingPartContainerSize = floatingPartContainer.size();
    for (unsigned int i = 0; i < floatingPartContainerSize; ++i) {
        floatingPart += floatingPartContainer[i] / std::pow(10, i + 1);
    }

    double result = 0;
    result = (double)(integerPart + floatingPart);

    if (eFlag) {
        if (eSign) {
            result /= std::pow(10, eNumber);
        } else {
            result *= std::pow(10, eNumber);
        }
    }

    if (negateFlag) {
        result *= -1.0f;
    }

    return result;
}

void CJsonParser::SearchInJsonArray(
    std::vector<JsonValue>* arrayValue,
    const char* key_,
    std::vector<JsonValue>& resultVector
) const {
    for (unsigned int i = 0; i < arrayValue->size(); ++i) {
        if ((*arrayValue)[i].type == JsonObject) {
            SearchInJsonObject(
                (*arrayValue)[i].value.object,
                key_,
                resultVector
            );
        }

        if ((*arrayValue)[i].type == JsonArray) {
            SearchInJsonArray((*arrayValue)[i].value.array, key_, resultVector);
        }
    }
}

void CJsonParser::SearchInJsonObject(
    HashMap<JsonValue>* mapValue,
    const char* key_,
    std::vector<JsonValue>& resultVector
) const {
    for (unsigned int i = 0; i < mapValue->GetCapacity(); ++i) {
        if (mapValue->hashMap_[i] != nullptr) {
            Node<JsonValue>* current = mapValue->hashMap_[i];
            while (current != nullptr) {
                std::string searchKey = key_;
                std::string currentKey = current->key_;
                if (currentKey == searchKey) {
                    resultVector.push_back(current->value_);
                }

                if (current->value_.type == JsonObject) {
                    SearchInJsonObject(
                        current->value_.value.object,
                        key_,
                        resultVector
                    );
                }

                if (current->value_.type == JsonArray) {
                    SearchInJsonArray(
                        current->value_.value.array,
                        key_,
                        resultVector
                    );
                }

                current = current->next_;
            }
        }
    }
}

std::vector<JsonValue> CJsonParser::Search(const char* key_) const {
    std::vector<JsonValue> resultVector;
    SearchInJsonObject(root_->value.object, key_, resultVector);
    return resultVector;
}

template<typename T>
bool isElementExist(const T element, const std::vector<T>& array) {
    for (uint32_t n = 0; n < array.size(); ++n) {
        if (element == array[n]) {
            return true;
        }
    }

    return false;
}

template<typename T>
T getElementIndex(const T element, const std::vector<T>& array) {
    for (uint32_t n = 0; n < array.size(); ++n) {
        if (element == array[n]) {
            return n;
        }
    }

    return std::numeric_limits<T>::max();
}

void calculateElementsMemorySize(
    const uint32_t indices_elements_count,
    std::string* indices_element_type,
    const uint32_t indices_componet_type,
    uint32_t* indices_buffer_view_byte_length
) {
    if (*indices_element_type == "VEC2") {
        if (indices_componet_type == 5120 || indices_componet_type == 5121) {
            *indices_buffer_view_byte_length = indices_elements_count * 2;
        } else if (indices_componet_type == 5122 || indices_componet_type == 5123) {
            *indices_buffer_view_byte_length = indices_elements_count * 4;
        } else if (indices_componet_type == 5125 || indices_componet_type == 5126) {
            *indices_buffer_view_byte_length = indices_elements_count * 8;
        }
    } else if (*indices_element_type == "VEC3") {
        if (indices_componet_type == 5120 || indices_componet_type == 5121) {
            *indices_buffer_view_byte_length = indices_elements_count * 3;
        } else if (indices_componet_type == 5122 || indices_componet_type == 5123) {
            *indices_buffer_view_byte_length = indices_elements_count * 6;
        } else if (indices_componet_type == 5125 || indices_componet_type == 5126) {
            *indices_buffer_view_byte_length = indices_elements_count * 12;
        }
    } else if (*indices_element_type == "VEC4") {
        if (indices_componet_type == 5120 || indices_componet_type == 5121) {
            *indices_buffer_view_byte_length = indices_elements_count * 4;
        } else if (indices_componet_type == 5122 || indices_componet_type == 5123) {
            *indices_buffer_view_byte_length = indices_elements_count * 8;
        } else if (indices_componet_type == 5125 || indices_componet_type == 5126) {
            *indices_buffer_view_byte_length = indices_elements_count * 16;
        }
    } else if (*indices_element_type == "SCALAR") {
        if (indices_componet_type == 5120 || indices_componet_type == 5121) {
            *indices_buffer_view_byte_length = indices_elements_count;
        } else if (indices_componet_type == 5122 || indices_componet_type == 5123) {
            *indices_buffer_view_byte_length = indices_elements_count * 2;
        } else if (indices_componet_type == 5125 || indices_componet_type == 5126) {
            *indices_buffer_view_byte_length = indices_elements_count * 4;
        }
    } else if (*indices_element_type == "MAT4") {
        *indices_buffer_view_byte_length = indices_elements_count * 64;
    }
}

struct ComponentType {
    enum Type {
        I8 = 5120,
        U8 = 5121,
        I16 = 5122,
        U16 = 5123,
        U32 = 5125,
        F32 = 5126
    };
};

void calculateByteStep(uint32_t componetType, unsigned int* byteStep) {
    if (componetType == ComponentType::I8
        || componetType == ComponentType::U8) {
        *byteStep = 1;
    } else if (
        componetType == ComponentType::I16 || componetType == ComponentType::U16
    ) {
        *byteStep = 2;
    } else if (
        componetType == ComponentType::U32 || componetType == ComponentType::F32
    ) {
        *byteStep = 4;
    }
}

// Metadata structs to binary buffer with actual data.
struct AccessorMetaData {
    uint32_t bufferView;
    uint32_t byteOffset;
    uint32_t componentType;
    uint32_t count;
    std::string type;
};

struct BufferViewMetaData {
    uint32_t byteLength;
    uint32_t byteOffset;
};

[[nodiscard]] AccessorMetaData readAccessorMetaData(
    JsonValue* gltf,
    const uint32_t accessorIndex
) {
    AccessorMetaData bufferMetaData;
    bufferMetaData.bufferView =
        (*gltf)["accessors"][accessorIndex]["bufferView"].value.iNumber;
    bufferMetaData.count =
        (*gltf)["accessors"][accessorIndex]["count"].value.iNumber;
    bufferMetaData.type =
        *(*gltf)["accessors"][accessorIndex]["type"].value.string;
    bufferMetaData.componentType =
        (*gltf)["accessors"][accessorIndex]["componentType"].value.iNumber;

    bufferMetaData.byteOffset = 0;
    if ((*gltf)["accessors"][accessorIndex].isObject() == JsonObject) {
        HashMap<JsonValue>* ptr =
            (*gltf)["accessors"][accessorIndex].value.object;
        if (ptr->Contain("byteOffset")) {
            bufferMetaData.byteOffset =
                (*gltf)["accessors"][accessorIndex]["byteOffset"].value.iNumber;
        }
    }

    return bufferMetaData;
}

[[nodiscard]] BufferViewMetaData readBufferViewMetaData(
    JsonValue* gltf,
    const uint32_t bufferViewIndex
) {
    BufferViewMetaData bufferViewMetaData;
    bufferViewMetaData.byteLength =
        (*gltf)["bufferViews"][bufferViewIndex]["byteLength"].value.iNumber;
    bufferViewMetaData.byteOffset = 0;
    if ((*gltf)["bufferViews"][bufferViewIndex].isObject() == JsonObject) {
        HashMap<JsonValue>* ptr =
            (*gltf)["bufferViews"][bufferViewIndex].value.object;
        if (ptr->Contain("byteOffset")) {
            bufferViewMetaData.byteOffset =
                (*gltf)["bufferViews"][bufferViewIndex]["byteOffset"]
                    .value.iNumber;
        }
    }

    return bufferViewMetaData;
}

template<typename T>
void readBinaryBufferData(
    char* buffer,
    AccessorMetaData accessorMetaData,
    BufferViewMetaData bufferViewMetaData,
    std::vector<T>& outputData
) {
    uint32_t indicesByteStep = 0;
    calculateByteStep(accessorMetaData.componentType, &indicesByteStep);

    unsigned int byteLength = 0;
    calculateElementsMemorySize(
        accessorMetaData.count,
        &accessorMetaData.type,
        accessorMetaData.componentType,
        &byteLength
    );
    for (unsigned int i =
             bufferViewMetaData.byteOffset + accessorMetaData.byteOffset;
         i < bufferViewMetaData.byteOffset + accessorMetaData.byteOffset
             + byteLength;
         i += indicesByteStep) {
        switch (accessorMetaData.componentType) {
            case ComponentType::U8:
                outputData.push_back(
                    reinterpret_cast<unsigned char&>(buffer[i])
                );
                break;
            case ComponentType::U16:
                outputData.push_back(
                    reinterpret_cast<unsigned short&>(buffer[i])
                );
                break;
            case ComponentType::U32:
                outputData.push_back(reinterpret_cast<unsigned int&>(buffer[i]));
                break;
            case ComponentType::F32:
                outputData.push_back(reinterpret_cast<float&>(buffer[i]));
                break;
        }
    }
}

void CJsonParser::LoadGLTF(
    const char* pathsGLTF_,
    std::vector<float>& aVertexes_,
    std::vector<uint32_t>& aIndices_,
    std::vector<std::vector<Matrix<float, 4>>>& jointMatricesPerMesh,
    std::vector<float>& frames,
    bool& noAnimations,
    float& topY
) {
    std::cout << "path: " << pathsGLTF_ << std::endl;
    ReadFile(pathsGLTF_);
    Parse();
    JsonValue* gltf = GetRoot();
    std::string binary_path = *(*gltf)["buffers"][0]["uri"].value.string;
    int full_byte_size = (*gltf)["buffers"][0]["byteLength"].value.iNumber;
    size_t lastSeparator = std::string(pathsGLTF_).find_last_of("/\\");
    std::string binary_full_path = lastSeparator == std::string::npos
        ? binary_path
        : std::string(pathsGLTF_).substr(0, lastSeparator + 1) + binary_path;
    std::ifstream in_stream;
    in_stream.open(binary_full_path, std::ios::binary);
    if (!in_stream.is_open()) {
        throw std::runtime_error(
            "failed to open gltf binary buffer: " + binary_full_path
        );
    }
    char* buffer = new char[full_byte_size];
    in_stream.read(buffer, full_byte_size);
    in_stream.close();
    const uint32_t indicesAccessorIndex =
        (*gltf)["meshes"][0]["primitives"][0]["indices"].value.iNumber;
    AccessorMetaData indicesAccessorMetaData =
        readAccessorMetaData(gltf, indicesAccessorIndex);
    BufferViewMetaData indicesBufferViewMetaData =
        readBufferViewMetaData(gltf, indicesAccessorMetaData.bufferView);
    std::vector<uint32_t> indices;
    readBinaryBufferData(
        buffer,
        indicesAccessorMetaData,
        indicesBufferViewMetaData,
        indices
    );
    const uint32_t verticesPositionAccessorIndex =
        (*gltf)["meshes"][0]["primitives"][0]["attributes"]["POSITION"]
            .value.iNumber;
    AccessorMetaData verticesPositionAccessorMetaData =
        readAccessorMetaData(gltf, verticesPositionAccessorIndex);
    BufferViewMetaData verticesPositionBufferViewMetaData =
        readBufferViewMetaData(
            gltf,
            verticesPositionAccessorMetaData.bufferView
        );
    std::vector<float> verticesPosition;
    readBinaryBufferData(
        buffer,
        verticesPositionAccessorMetaData,
        verticesPositionBufferViewMetaData,
        verticesPosition
    );
    const uint32_t textureCoordinatesAccessorIndex =
        (*gltf)["meshes"][0]["primitives"][0]["attributes"]["TEXCOORD_0"]
            .value.iNumber;
    AccessorMetaData textureCoordinatesAccessorMetaData =
        readAccessorMetaData(gltf, textureCoordinatesAccessorIndex);
    BufferViewMetaData textureCoordinatesBufferViewMetaData =
        readBufferViewMetaData(
            gltf,
            textureCoordinatesAccessorMetaData.bufferView
        );
    std::vector<float> textureCoordinates;
    readBinaryBufferData(
        buffer,
        textureCoordinatesAccessorMetaData,
        textureCoordinatesBufferViewMetaData,
        textureCoordinates
    );
    const uint32_t normalsAccessorIndex =
        (*gltf)["meshes"][0]["primitives"][0]["attributes"]["NORMAL"]
            .value.iNumber;
    AccessorMetaData normalsAccessorMetaData =
        readAccessorMetaData(gltf, normalsAccessorIndex);
    BufferViewMetaData normalsBufferViewMetaData =
        readBufferViewMetaData(gltf, normalsAccessorMetaData.bufferView);
    std::vector<float> normals;
    readBinaryBufferData(
        buffer,
        normalsAccessorMetaData,
        normalsBufferViewMetaData,
        normals
    );
    std::vector<JsonValue> skins = Search("skins");
    JsonValue joints;
    std::vector<Matrix<float, 4>> globalTransformJointNode;
    std::vector<Matrix<float, 4>> inverseBindMatrixSet;
    std::vector<std::vector<Matrix<float, 4>>> jointMatrices;
    std::vector<float> weightsContainer;
    std::vector<int> jointsIndices;
    std::vector<std::vector<int>> children;
    if (skins.size() > 0) {
        noAnimations = false;
        joints = (*gltf)["skins"][0]["joints"];
        JsonValue nodes = (*gltf)["nodes"];
        // Loop on joints.
        for (unsigned int i = 0; i < joints.value.array->size(); ++i) {
            unsigned int jointIndexMapToNode =
                (*joints.value.array)[i].value.iNumber;
            JsonValue node = nodes[jointIndexMapToNode];
            Quaternion rotationQuaternion;
            Matrix<float, 4> rotation(1.0f);
            Matrix<float, 4> scale(1.0f);
            Matrix<float, 4> translation(1.0f);
            if (node.value.object->Contain("rotation")) {
                JsonValue array = (*node.value.object)["rotation"];
                for (unsigned int i = 0; i < array.value.array->size(); ++i) {
                    switch (i) {
                        case 0:
                            if (array[i].isInterger()) {
                                rotationQuaternion.x = array[i].value.iNumber;
                            } else if (array[i].isFloat()) {
                                rotationQuaternion.x = array[i].value.fNumber;
                            }
                            break;
                        case 1:
                            if (array[i].isInterger()) {
                                rotationQuaternion.y = array[i].value.iNumber;
                            } else if (array[i].isFloat()) {
                                rotationQuaternion.y = array[i].value.fNumber;
                            }
                            break;
                        case 2:
                            if (array[i].isInterger()) {
                                rotationQuaternion.z = array[i].value.iNumber;
                            } else if (array[i].isFloat()) {
                                rotationQuaternion.z = array[i].value.fNumber;
                            }
                            break;
                        case 3:
                            if (array[i].isInterger()) {
                                rotationQuaternion.w = array[i].value.iNumber;
                            } else if (array[i].isFloat()) {
                                rotationQuaternion.w = array[i].value.fNumber;
                            }
                            break;
                    }
                }
                rotation = rotateQuaternion<float, 4>(rotationQuaternion);
                rotation.SelfTensorTranspose();
            }
            std::vector<int> local_children;
            // Collect children indices.
            if (node.value.object->Contain("children")) {
                JsonValue array = (*node.value.object)["children"];
                for (unsigned int i = 0; i < array.value.array->size(); ++i) {
                    local_children.push_back(array[i].value.iNumber);
                }
                // Linearly put all children to every root joint.
                children.push_back(local_children);
            } else {
                std::vector<int> emptyChildren;
                // Put empty pack of children if can find a one.
                children.push_back(emptyChildren);
            }
            if (node.value.object->Contain("scale")) {
                JsonValue array = (*node.value.object)["scale"];
                for (unsigned int i = 0; i < array.value.array->size(); ++i) {
                    if (array[i].isInterger()) {
                        scale[i][i] = array[i].value.iNumber;
                    } else if (array[i].isFloat()) {
                        scale[i][i] = array[i].value.fNumber;
                    }
                }
            }
            if (node.value.object->Contain("translation")) {
                JsonValue array = (*node.value.object)["translation"];
                for (unsigned int i = 0; i < array.value.array->size(); ++i) {
                    if (array[i].isInterger()) {
                        translation[3][i] = array[i].value.iNumber;
                    } else if (array[i].isFloat()) {
                        translation[3][i] = array[i].value.fNumber;
                    }
                }
            }
            // Compute model matrix.
            Matrix<float, 4> model = scale * rotation * translation;
            globalTransformJointNode.push_back(model);
        }
        // Get the inverse bind matrices accessor index.
        const uint32_t inverseBindMatricesAccessorIndex =
            (*gltf)["skins"][0]["inverseBindMatrices"].value.iNumber;
        AccessorMetaData inverseBindMatricesAccessorMetaData =
            readAccessorMetaData(gltf, inverseBindMatricesAccessorIndex);
        BufferViewMetaData inveresBindMatricesBufferViewMetaData =
            readBufferViewMetaData(
                gltf,
                inverseBindMatricesAccessorMetaData.bufferView
            );
        std::vector<float> inverseBindMatricesData;
        readBinaryBufferData(
            buffer,
            inverseBindMatricesAccessorMetaData,
            inveresBindMatricesBufferViewMetaData,
            inverseBindMatricesData
        );
        Matrix<float, 4> inverseBindMatrix(0.0f);
        for (unsigned int n = 0; n < joints.value.array->size(); ++n) {
            for (unsigned int g = 0; g < 4; ++g) {
                for (unsigned int j = 0; j < 4; ++j) {
                    // Put row float data into mat4.
                    inverseBindMatrix[g][j] =
                        inverseBindMatricesData[n * 16 + g * 4 + j];
                }
            }
            inverseBindMatrixSet.push_back(inverseBindMatrix);
        }
        const uint32_t jointsAccessorIndex =
            (*gltf)["meshes"][0]["primitives"][0]["attributes"]["JOINTS_0"]
                .value.iNumber;
        AccessorMetaData jointsAccessorMetaData =
            readAccessorMetaData(gltf, jointsAccessorIndex);
        BufferViewMetaData jointsBufferViewMetaData =
            readBufferViewMetaData(gltf, jointsAccessorMetaData.bufferView);
        readBinaryBufferData(
            buffer,
            jointsAccessorMetaData,
            jointsBufferViewMetaData,
            jointsIndices
        );
        unsigned int weightsAccessorIndex =
            (*gltf)["meshes"][0]["primitives"][0]["attributes"]["WEIGHTS_0"]
                .value.iNumber;
        AccessorMetaData weightsAccessorMetaData =
            readAccessorMetaData(gltf, weightsAccessorIndex);
        BufferViewMetaData weightsBufferViewMetaData =
            readBufferViewMetaData(gltf, weightsAccessorMetaData.bufferView);
        readBinaryBufferData(
            buffer,
            weightsAccessorMetaData,
            weightsBufferViewMetaData,
            weightsContainer
        );
    } else {
        noAnimations = true;
    }
    std::vector<JsonValue> animations = Search("animations");
    if (animations.size() > 0) {
        std::vector<JsonValue> samplerIndices;
        std::vector<JsonValue> targetNodes;
        std::vector<JsonValue> targetPaths;
        JsonValue channels = (*gltf)["animations"][0]["channels"];
        for (unsigned int i = 0; i < channels.value.array->size(); ++i) {
            samplerIndices.push_back(channels[i]["sampler"]);
        }
        for (unsigned int i = 0; i < channels.value.array->size(); ++i) {
            targetNodes.push_back(channels[i]["target"]["node"]);
        }
        for (unsigned int i = 0; i < channels.value.array->size(); ++i) {
            targetPaths.push_back(channels[i]["target"]["path"]);
        }
        std::vector<unsigned int> translationSamplerIndices;
        std::vector<unsigned int> rotationSamplerIndices;
        std::vector<unsigned int> scaleSamplerIndices;
        std::vector<uint32_t> nodesMapTranslations;
        std::vector<uint32_t> nodesMapRotations;
        std::vector<uint32_t> nodesMapScales;
        for (unsigned int i = 0; i < samplerIndices.size(); ++i) {
            if (*targetPaths[i].value.string == "translation") {
                translationSamplerIndices.push_back(
                    samplerIndices[i].value.iNumber
                );
                nodesMapTranslations.push_back(targetNodes[i].value.iNumber);
            } else if (*targetPaths[i].value.string == "rotation") {
                rotationSamplerIndices.push_back(
                    samplerIndices[i].value.iNumber
                );
                nodesMapRotations.push_back(targetNodes[i].value.iNumber);
            } else if (*targetPaths[i].value.string == "scale") {
                scaleSamplerIndices.push_back(samplerIndices[i].value.iNumber);
                nodesMapScales.push_back(targetNodes[i].value.iNumber);
            }
        }
        JsonValue samplers = (*gltf)["animations"][0]["samplers"];
        std::vector<unsigned int> translationInputs;
        std::vector<unsigned int> translationOutputs;
        for (unsigned int i = 0; i < translationSamplerIndices.size(); ++i) {
            translationInputs.push_back(
                samplers[translationSamplerIndices[i]]["input"].value.iNumber
            );
        }
        for (unsigned int i = 0; i < translationSamplerIndices.size(); ++i) {
            translationOutputs.push_back(
                samplers[translationSamplerIndices[i]]["output"].value.iNumber
            );
        }
        std::vector<std::vector<float>> frameInputsTranslation;
        for (unsigned int i = 0; i < translationInputs.size(); ++i) {
            AccessorMetaData frameInputsTranslationAccessorMetaData =
                readAccessorMetaData(gltf, translationInputs[i]);
            BufferViewMetaData frameInputsTranslationBufferViewMetaData =
                readBufferViewMetaData(
                    gltf,
                    frameInputsTranslationAccessorMetaData.bufferView
                );
            std::vector<float> temp;
            readBinaryBufferData(
                buffer,
                frameInputsTranslationAccessorMetaData,
                frameInputsTranslationBufferViewMetaData,
                temp
            );
            frameInputsTranslation.push_back(temp);
        }
        std::vector<std::vector<float>> translations;
        for (unsigned int i = 0; i < translationOutputs.size(); ++i) {
            AccessorMetaData frameOutputsTranslationAccessorMetaData =
                readAccessorMetaData(gltf, translationOutputs[i]);
            BufferViewMetaData frameOutputsTranslationBufferViewMetaData =
                readBufferViewMetaData(
                    gltf,
                    frameOutputsTranslationAccessorMetaData.bufferView
                );
            std::vector<float> temp;
            readBinaryBufferData(
                buffer,
                frameOutputsTranslationAccessorMetaData,
                frameOutputsTranslationBufferViewMetaData,
                temp
            );
            translations.push_back(temp);
        }
        std::vector<unsigned int> rotationInputs;
        std::vector<unsigned int> rotationOutputs;
        for (unsigned int i = 0; i < rotationSamplerIndices.size(); ++i) {
            rotationInputs.push_back(
                samplers[rotationSamplerIndices[i]]["input"].value.iNumber
            );
        }
        for (unsigned int i = 0; i < rotationSamplerIndices.size(); ++i) {
            rotationOutputs.push_back(
                samplers[rotationSamplerIndices[i]]["output"].value.iNumber
            );
        }
        std::vector<std::vector<float>> frameInputsRotation;
        for (unsigned int i = 0; i < rotationInputs.size(); ++i) {
            AccessorMetaData frameInputsRotationAccessorMetaData =
                readAccessorMetaData(gltf, rotationInputs[i]);
            BufferViewMetaData frameInputsRotationBufferViewMetaData =
                readBufferViewMetaData(
                    gltf,
                    frameInputsRotationAccessorMetaData.bufferView
                );
            std::vector<float> temp;
            readBinaryBufferData(
                buffer,
                frameInputsRotationAccessorMetaData,
                frameInputsRotationBufferViewMetaData,
                temp
            );
            frameInputsRotation.push_back(temp);
        }
        std::vector<std::vector<float>> rotations;
        for (unsigned int i = 0; i < rotationOutputs.size(); ++i) {
            AccessorMetaData frameOutputsRotationAccessorMetaData =
                readAccessorMetaData(gltf, rotationOutputs[i]);
            BufferViewMetaData frameOutputsRotationBufferViewMetaData =
                readBufferViewMetaData(
                    gltf,
                    frameOutputsRotationAccessorMetaData.bufferView
                );
            std::vector<float> temp;
            readBinaryBufferData(
                buffer,
                frameOutputsRotationAccessorMetaData,
                frameOutputsRotationBufferViewMetaData,
                temp
            );
            rotations.push_back(temp);
        }
        std::vector<unsigned int> scaleInputs;
        std::vector<unsigned int> scaleOutputs;
        for (unsigned int i = 0; i < scaleSamplerIndices.size(); ++i) {
            scaleInputs.push_back(
                samplers[scaleSamplerIndices[i]]["input"].value.iNumber
            );
        }
        for (unsigned int i = 0; i < scaleSamplerIndices.size(); ++i) {
            scaleOutputs.push_back(
                samplers[scaleSamplerIndices[i]]["output"].value.iNumber
            );
        }
        std::vector<std::vector<float>> frameInputsScale;
        for (unsigned int i = 0; i < scaleInputs.size(); ++i) {
            AccessorMetaData frameInputsScaleAccessorMetaData =
                readAccessorMetaData(gltf, scaleInputs[i]);
            BufferViewMetaData frameInputsScaleBufferViewMetaData =
                readBufferViewMetaData(
                    gltf,
                    frameInputsScaleAccessorMetaData.bufferView
                );
            std::vector<float> temp;
            readBinaryBufferData(
                buffer,
                frameInputsScaleAccessorMetaData,
                frameInputsScaleBufferViewMetaData,
                temp
            );
            frameInputsScale.push_back(temp);
        }
        std::vector<std::vector<float>> scales;
        for (unsigned int i = 0; i < scaleOutputs.size(); ++i) {
            AccessorMetaData frameOutputsScaleAccessorMetaData =
                readAccessorMetaData(gltf, scaleOutputs[i]);
            BufferViewMetaData frameOutputsScaleBufferViewMetaData =
                readBufferViewMetaData(
                    gltf,
                    frameOutputsScaleAccessorMetaData.bufferView
                );
            std::vector<float> temp;
            readBinaryBufferData(
                buffer,
                frameOutputsScaleAccessorMetaData,
                frameOutputsScaleBufferViewMetaData,
                temp
            );
            scales.push_back(temp);
        }
        // Searching for root joins.
        std::vector<int> rootNodes;
        for (unsigned int s = 0; s < joints.value.array->size(); ++s) {
            int current_joint = (*joints.value.array)[s].value.iNumber;
            for (unsigned w = 0; w < children.size(); ++w) {
                for (unsigned q = 0; q < children[w].size(); ++q) {
                    if (children[w][q] == current_joint) {
                        goto most_scary_operator_of_all_time;
                    }
                }
            }
            // If we execute this line then this joint index ectualy the root.
            rootNodes.push_back(current_joint);
        most_scary_operator_of_all_time: // Not so scary at all. Am i right?
            continue;
        }
        std::vector<std::vector<unsigned int>> nodesHierarchy;
        // Loop on parent joints.
        for (unsigned int w = 0; w < rootNodes.size(); ++w) {
            std::vector<std::vector<unsigned int>> nodes_bones;
            unsigned int currentRoot = rootNodes[w];
            std::vector<uint32_t> node_stack;
            // Start from root joint.
            node_stack.push_back(currentRoot);
            std::vector<uint32_t> deepness_stack;
            traversalBones(
                children,
                joints,
                node_stack,
                deepness_stack,
                nodes_bones
            );
            for (unsigned int e = 0; e < nodes_bones.size(); ++e) {
                nodesHierarchy.push_back(nodes_bones[e]);
            }
        }
        // This logic related to joints that has inverseBindMatrices.
        [[maybe_unused]] uint32_t transformationsMax =
            translations.size() > scales.size()
            ? (translations.size() > rotations.size() ? translations.size()
                                                      : rotations.size())
            : (scales.size() > rotations.size() ? scales.size()
                                                : rotations.size());
        const uint32_t numJoints = joints.value.array->size();
        uint32_t translationFramesNumber = 0;
        for (uint32_t k = 0; k < frameInputsTranslation.size(); ++k) {
            if (frameInputsTranslation[k].size() > translationFramesNumber) {
                translationFramesNumber = frameInputsTranslation[k].size();
            }
        }
        uint32_t rotationFramesNumber = 0;
        for (uint32_t k = 0; k < frameInputsRotation.size(); ++k) {
            if (frameInputsRotation[k].size() > rotationFramesNumber) {
                rotationFramesNumber = frameInputsRotation[k].size();
            }
        }
        uint32_t scaleFramesNumber = 0;
        for (uint32_t k = 0; k < frameInputsScale.size(); ++k) {
            if (frameInputsScale[k].size() > scaleFramesNumber) {
                scaleFramesNumber = frameInputsScale[k].size();
            }
        }
        const uint32_t framesMax = translationFramesNumber > scaleFramesNumber
            ? (translationFramesNumber > rotationFramesNumber
                   ? translationFramesNumber
                   : rotationFramesNumber)
            : (scaleFramesNumber > rotationFramesNumber ? scaleFramesNumber
                                                        : rotationFramesNumber);
        for (uint32_t k = 0; k < frameInputsTranslation.size(); ++k) {
            if (frameInputsTranslation[k].size() > frames.size()) {
                frames = frameInputsTranslation[k];
            }
        }
        for (uint32_t k = 0; k < frameInputsRotation.size(); ++k) {
            if (frameInputsRotation[k].size() > frames.size()) {
                frames = frameInputsRotation[k];
            }
        }
        for (uint32_t k = 0; k < frameInputsScale.size(); ++k) {
            if (frameInputsScale[k].size() > frames.size()) {
                frames = frameInputsScale[k];
            }
        }
        std::vector<int> jointToTranslationCh;
        std::vector<int> jointToRotationCh;
        std::vector<int> jointToScaleCh;
        for (uint32_t k = 0; k < numJoints; ++k) {
            jointToTranslationCh.push_back(-1);
            jointToRotationCh.push_back(-1);
            jointToScaleCh.push_back(-1);
        }
        for (uint32_t k = 0; k < nodesMapTranslations.size(); ++k) {
            uint32_t jIdx =
                getJointIndex(joints, (int32_t)nodesMapTranslations[k]);
            if (jIdx != UINT32_MAX) {
                jointToTranslationCh[jIdx] = (int)k;
            }
        }
        for (uint32_t k = 0; k < nodesMapRotations.size(); ++k) {
            uint32_t jIdx =
                getJointIndex(joints, (int32_t)nodesMapRotations[k]);
            if (jIdx != UINT32_MAX) {
                jointToRotationCh[jIdx] = (int)k;
            }
        }
        for (uint32_t k = 0; k < nodesMapScales.size(); ++k) {
            uint32_t jIdx = getJointIndex(joints, (int32_t)nodesMapScales[k]);
            if (jIdx != UINT32_MAX) {
                jointToScaleCh[jIdx] = (int)k;
            }
        }
        // Build animatedNodesMatricesAccumulator indexed by joint-index
        // (0..numJoints - 1).
        std::vector<std::vector<Matrix<float, 4>>>
            animatedNodesMatricesAccumulator;
        for (unsigned int j = 0; j < numJoints; ++j) {
            int tIdx = jointToTranslationCh[j];
            int rIdx = jointToRotationCh[j];
            int sIdx = jointToScaleCh[j];
            // Local defaults fresh on every joint, for not make possible to
            // collect data from previous iterations.
            std::vector<float> defaultTranslations;
            std::vector<float> defaultRotations;
            std::vector<float> defaultScales;
            // Static TRS from node. Using if channel not exists.
            int32_t nodeIdx = (int32_t)(*joints.value.array)[j].value.iNumber;
            float sTx = 0.f, sTy = 0.f, sTz = 0.f;
            float sRx = 0.f, sRy = 0.f, sRz = 0.f, sRw = 1.f;
            float sSx = 1.f, sSy = 1.f, sSz = 1.f;
            if ((*gltf)["nodes"][nodeIdx].isObject() == JsonObject) {
                auto* nd = (*gltf)["nodes"][nodeIdx].value.object;
                if (nd->Contain("translation")) {
                    sTx = (*gltf)["nodes"][nodeIdx]["translation"][0]
                              .value.fNumber;
                    sTy = (*gltf)["nodes"][nodeIdx]["translation"][1]
                              .value.fNumber;
                    sTz = (*gltf)["nodes"][nodeIdx]["translation"][2]
                              .value.fNumber;
                }
                if (nd->Contain("rotation")) {
                    sRx =
                        (*gltf)["nodes"][nodeIdx]["rotation"][0].value.fNumber;
                    sRy =
                        (*gltf)["nodes"][nodeIdx]["rotation"][1].value.fNumber;
                    sRz =
                        (*gltf)["nodes"][nodeIdx]["rotation"][2].value.fNumber;
                    sRw =
                        (*gltf)["nodes"][nodeIdx]["rotation"][3].value.fNumber;
                }
                if (nd->Contain("scale")) {
                    sSx = (*gltf)["nodes"][nodeIdx]["scale"][0].value.fNumber;
                    sSy = (*gltf)["nodes"][nodeIdx]["scale"][1].value.fNumber;
                    sSz = (*gltf)["nodes"][nodeIdx]["scale"][2].value.fNumber;
                }
            }
            if (tIdx < 0) {
                for (uint32_t f = 0; f < framesMax; ++f) {
                    defaultTranslations.push_back(sTx);
                    defaultTranslations.push_back(sTy);
                    defaultTranslations.push_back(sTz);
                }
            }
            if (rIdx < 0) {
                for (uint32_t f = 0; f < framesMax; ++f) {
                    defaultRotations.push_back(sRx);
                    defaultRotations.push_back(sRy);
                    defaultRotations.push_back(sRz);
                    defaultRotations.push_back(sRw);
                }
            }
            if (sIdx < 0) {
                for (uint32_t f = 0; f < framesMax; ++f) {
                    defaultScales.push_back(sSx);
                    defaultScales.push_back(sSy);
                    defaultScales.push_back(sSz);
                }
            }
            std::vector<float>& boneT =
                (tIdx >= 0) ? translations[tIdx] : defaultTranslations;
            std::vector<float>& boneR =
                (rIdx >= 0) ? rotations[rIdx] : defaultRotations;
            std::vector<float>& boneS =
                (sIdx >= 0) ? scales[sIdx] : defaultScales;
            // Chennels can has verious number of frames; framesMax - gloabal
            // maximum. Clamp index to last valid chennel frame, for not run out
            // after vectors bounds.
            const uint32_t tFrames = boneT.size() / 3;
            const uint32_t rFrames = boneR.size() / 4;
            const uint32_t sFrames = boneS.size() / 3;
            std::vector<Matrix<float, 4>> perFrameMatrices;
            for (unsigned int i = 0; i < framesMax; ++i) {
                if (tFrames == 0 || rFrames == 0 || sFrames == 0) {
                    // Malformed data; skip joint.
                    std::vector<Matrix<float, 4>> empty;
                    animatedNodesMatricesAccumulator.push_back(empty);
                    continue;
                }
                const uint32_t ti = (i < tFrames) ? i : tFrames - 1;
                const uint32_t ri = (i < rFrames) ? i : rFrames - 1;
                const uint32_t si = (i < sFrames) ? i : sFrames - 1;
                Matrix<float, 4> frameTranslation(1.0f);
                Matrix<float, 4> frameScale(1.0f);
                for (unsigned int q = 0; q < 3; ++q) {
                    frameTranslation[3][q] = boneT[ti * 3 + q];
                    frameScale[q][q] = boneS[si * 3 + q];
                }
                Quaternion frameRotationQuaternion;
                Matrix<float, 4> frameRotation(1.0f);
                frameRotationQuaternion.x = boneR[ri * 4];
                frameRotationQuaternion.y = boneR[ri * 4 + 1];
                frameRotationQuaternion.z = boneR[ri * 4 + 2];
                frameRotationQuaternion.w = boneR[ri * 4 + 3];
                frameRotation =
                    rotateQuaternion<float, 4>(frameRotationQuaternion);
                frameRotation.SelfTensorTranspose();
                Matrix<float, 4> localTransform =
                    frameScale * frameRotation * frameTranslation;
                perFrameMatrices.push_back(localTransform);
            }
            animatedNodesMatricesAccumulator.push_back(perFrameMatrices);
        }
        // Final comstruction of joint-matrices. Both arrays indexed by
        // joint-index now, that's why nodesHierarchy[j][b] address accumulator
        // correctly.
        for (unsigned int j = 0; j < numJoints; ++j) {
            std::vector<Matrix<float, 4>> globalAllFrameNodeMatrix;
            for (unsigned int i = 0; i < framesMax; ++i) {
                Matrix<float, 4> rootTransform(1.0f);
                for (unsigned int b = 0; b < nodesHierarchy[j].size() - 1;
                     ++b) {
                    rootTransform =
                        animatedNodesMatricesAccumulator[nodesHierarchy[j][b]][i]
                        * rootTransform;
                }
                globalAllFrameNodeMatrix.push_back(
                    inverseBindMatrixSet[j]
                    * animatedNodesMatricesAccumulator[j][i] * rootTransform
                );
            }
            jointMatrices.push_back(globalAllFrameNodeMatrix);
        }
    }
    jointMatricesPerMesh = jointMatrices;
    topY = -999.999f;
    for (uint32_t i = 0; i < indices.size(); ++i) {
        aIndices_.push_back(i);
        unsigned int index = indices[i] * 3;
        if (index + 2 < verticesPosition.size()) {
            Vector<float, 3> position = {
                verticesPosition[index],
                verticesPosition[index + 1],
                verticesPosition[index + 2]
            };
            if (position[1] > topY) {
                topY = position[1];
            }
            aVertexes_.push_back(position[0]);
            aVertexes_.push_back(position[1]);
            aVertexes_.push_back(position[2]);
        }
        if (index + 2 < normals.size()) {
            Vector<float, 3> normal =
                {normals[index], normals[index + 1], normals[index + 2]};
            aVertexes_.push_back(normal[0]);
            aVertexes_.push_back(normal[1]);
            aVertexes_.push_back(normal[2]);
        }
        index = indices[i] * 2;
        if (index + 1 < textureCoordinates.size()) {
            aVertexes_.push_back(textureCoordinates[index]);
            aVertexes_.push_back(textureCoordinates[index + 1]);
        }
        index = indices[i] * 4;
        if (index + 3 < jointsIndices.size()) {
            aVertexes_.push_back(jointsIndices[index]);
            aVertexes_.push_back(jointsIndices[index + 1]);
            aVertexes_.push_back(jointsIndices[index + 2]);
            aVertexes_.push_back(jointsIndices[index + 3]);
        }
        if (index + 3 < weightsContainer.size()) {
            aVertexes_.push_back(weightsContainer[index]);
            aVertexes_.push_back(weightsContainer[index + 1]);
            aVertexes_.push_back(weightsContainer[index + 2]);
            aVertexes_.push_back(weightsContainer[index + 3]);
        }
    }
    delete[] buffer;
    buffer = nullptr;
}

void CJsonParser::traversalBones(
    std::vector<std::vector<int>> children,
    JsonValue joints,
    std::vector<uint32_t> node_stack,
    std::vector<uint32_t> deepness_stack,
    std::vector<std::vector<uint32_t>>& result
) {
    uint32_t topJointIndex = 0;
    if (!node_stack.empty()) {
        // Pass array of all joints and root joint and return index of root
        // joint in array.
        topJointIndex = getJointIndex(joints, node_stack.back());
    }
    if (node_stack.size() > deepness_stack.size()) {
        // First 0 level start from.
        uint32_t firstChild = 0;
        deepness_stack.push_back(firstChild);
    }
    // Main exit check.
    if (deepness_stack.empty()) {
        return;
    }
    uint32_t nextNodeIndex = 0;
    // Check current root joint has any children. Children maps linearly with
    // root joint array index.
    if (topJointIndex != UINT32_MAX && !children[topJointIndex].empty()) {
        // Check if on last child level.
        if (deepness_stack.back() > 0
            && deepness_stack.back() == children[topJointIndex].size()) {
            deepness_stack.pop_back();
            node_stack.pop_back();
            traversalBones(children, joints, node_stack, deepness_stack, result);
            return;
        }
        // Check if not on last child level.
        if (deepness_stack.back() > 0
            && deepness_stack.back() < children[topJointIndex].size()) {
            nextNodeIndex = children[topJointIndex][deepness_stack.back()];
            node_stack.push_back(nextNodeIndex);
            std::vector<uint32_t> current_node_indices;
            for (uint32_t i = 0; i < node_stack.size(); ++i) {
                uint32_t currentJoinIndex =
                    getJointIndex(joints, node_stack[i]);
                current_node_indices.push_back(currentJoinIndex);
            }
            ++deepness_stack.back();
            traversalBones(children, joints, node_stack, deepness_stack, result);
            return;
        } else {
            std::vector<uint32_t> current_node_indices;
            for (uint32_t i = 0; i < node_stack.size(); ++i) {
                uint32_t currentJoinIndex =
                    getJointIndex(joints, node_stack[i]);
                current_node_indices.push_back(currentJoinIndex);
            }
            result.push_back(current_node_indices);
            nextNodeIndex = children[topJointIndex][deepness_stack.back()];
            node_stack.push_back(nextNodeIndex);
            ++deepness_stack.back();
            traversalBones(children, joints, node_stack, deepness_stack, result);
            return;
        }
    } else {
        std::vector<uint32_t> current_node_indices;
        if (topJointIndex == UINT32_MAX) {
            current_node_indices.push_back(node_stack.back());
            result.push_back(current_node_indices);
            return;
        }
        for (uint32_t i = 0; i < node_stack.size(); ++i) {
            uint32_t currentJoinIndex = getJointIndex(joints, node_stack[i]);
            current_node_indices.push_back(currentJoinIndex);
        }
        result.push_back(current_node_indices);
        deepness_stack.pop_back();
        node_stack.pop_back();
        traversalBones(children, joints, node_stack, deepness_stack, result);
        return;
    }
}

std::vector<std::vector<unsigned int>> CJsonParser::makeRenderJointsIndices(
    std::vector<std::vector<unsigned int>>& input
) {
    std::vector<std::vector<unsigned int>> result;
    bool accumulatorFlag = false;
    bool innerFlag = false;
    unsigned int accumulator = input[0][0];
    for (unsigned int i = 0; i < input.size(); ++i) {
        for (unsigned int j = 0; j < input[i].size(); ++j) {
            std::vector<unsigned int> inner;
            for (unsigned int v = 0; v < j + 1; ++v) {
                if (input[i][j] == accumulator && accumulatorFlag) {
                    innerFlag = false;
                    continue;
                } else {
                    innerFlag = true;
                    inner.push_back(input[i][v]);

                    if (accumulatorFlag == false) {
                        accumulatorFlag = true;
                    }
                }
            }
            if (innerFlag) {
                result.push_back(inner);
            }
        }
    }
    return result;
}

bool CJsonParser::containsElement(
    std::vector<std::vector<unsigned int>> container,
    unsigned int element
) {
    bool flag = false;
    for (unsigned int i = 0; i < container.size(); ++i) {
        for (unsigned int j = 0; j < container[i].size(); ++j) {
            if (container[i][j] == element) {
                return true;
            }
        }
    }
    return flag;
}

uint32_t CJsonParser::getJointIndex(JsonValue joints, int32_t searchingIndex) {
    for (unsigned int i = 0; i < joints.value.array->size(); ++i) {
        int currentJointIndex = (*joints.value.array)[i].value.iNumber;
        if (currentJointIndex == searchingIndex) {
            return i;
        }
    }
    return -1;
}

CJsonParser::~CJsonParser() {
    delete root_;
}
} // namespace glvm

namespace glvm {
MeshManager* MeshManager::pInstance_ = nullptr;
std::mutex MeshManager::Mutex_;

MeshManager::MeshManager() {}

MeshManager::~MeshManager() {}

void MeshManager::SetMesh(const char* _pathToMesh) {
    pathsArray_.push_back(_pathToMesh);
}

void MeshManager::SetMeshGLTF(const char* pathToMesh) {
    pathsGLTF_.push_back(pathToMesh);
}

MeshManager* MeshManager::get_instance() {
    std::lock_guard<std::mutex> lock(Mutex_);
    if (pInstance_ == nullptr) {
        pInstance_ = new MeshManager();
    }
    return pInstance_;
}
} // namespace glvm

namespace glvm {
void ProceduralLevelGeneratingSystem::Update() {
    using namespace glvm;
    Engine* GLVM = Engine::get_instance();

    // New arch ECS.
    ArchetypeEntityManager* archEntityManager =
        ArchetypeEntityManager::get_instance();

    world.searchCacheArchetypes(
        playerRequiredMask,
        &archView.cachedPlayerArch,
        cachedPlayerArchNumber
    );
    componentsView.playerTransforms =
        (transform*)archView.cachedPlayerArch
            ->components[ComponentsIndices::TransformComponent];

    while (levelNubmer < 5) {
        std::vector<Vertex> nextLevel;
        std::vector<uint32_t> indices;
        std::vector<Vertex> transitionBridgeVertices;
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

            [[maybe_unused]] MeshHandle gameLevelMeshHandle = GLVM->LoadMesh();
            uint64_t gameLevelChunkEntity = archEntityManager->createEntity();

            cachedLevelChunkArchNumber = 0;
            // Search and cache one time for LevelChunkArch.
            world.searchCacheArchetypes(
                requiredMask,
                &archView.cachedLevelChunkArch,
                cachedLevelChunkArchNumber
            );

            world.addEntityToArchetype(
                gameLevelChunkEntity,
                archView.cachedLevelChunkArch
            );
            EntityLocation gameLevelChunkLocation =
                world.entityLocations[getId(gameLevelChunkEntity)];

            LevelChunkArchetype* levelChunkArch =
                static_cast<LevelChunkArchetype*>(gameLevelChunkLocation.arch);
            const uint32_t gameLevelChunkIndex = gameLevelChunkLocation.index;
            TextureHandle gameLevelTexture = textureHandlers[2];
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

            [[maybe_unused]] MeshHandle transitionBridgeMeshHandle =
                GLVM->LoadMesh();
            uint64_t transitionBridgeEntity = archEntityManager->createEntity();
            world.addEntityToArchetype(
                transitionBridgeEntity,
                archView.cachedLevelChunkArch
            );

            EntityLocation transitionBridgeLocation =
                world.entityLocations[getId(transitionBridgeEntity)];

            LevelChunkArchetype* transitionBridgeArch =
                static_cast<LevelChunkArchetype*>(transitionBridgeLocation.arch);
            const uint32_t transitionBridgeIndex =
                transitionBridgeLocation.index;
            TextureHandle transitionBridgeTexture = textureHandlers[3];
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
    std::vector<Vertex>& destinationVerticesContainer
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
    return position[0] + half_x > coordinateMaximumValuePerDirection.lowest_x
        && position[0] - half_x < coordinateMaximumValuePerDirection.highest_x
        && position[1] + half_y > coordinateMaximumValuePerDirection.lowest_y
        && position[1] - half_y < coordinateMaximumValuePerDirection.highest_y
        && position[2] + half_z > coordinateMaximumValuePerDirection.lowest_z
        && position[2] - half_z < coordinateMaximumValuePerDirection.highest_z;
}
} // namespace glvm

#ifdef __linux__
#endif

#ifdef _WIN32
#endif

namespace glvm {
ISoundEngine* CSoundEngineFactory::CreateSoundEngine() {
#ifdef __linux__
    return new CSoundEngineAlsa;
#endif

#ifdef _WIN32
    return new CSoundEngineWaveform;
#endif
}

void CSoundEngineWaveform::OpenDevice(const char* /* device */) {}

void CSoundEngineWaveform::CloseDevice() {}

void CSoundEngineWaveform::CreateSoundSample(
    const char* filePath,
    uint32_t duration,
    uint32_t rate,
    float volume
) {
    CSoundSample* sample = new CSoundSample {filePath, duration, rate, volume};
    tSound_Container.push_back(sample);
}
} // namespace glvm

namespace glvm {
CSystemManager* CSystemManager::pInstance_ = nullptr;
std::mutex CSystemManager::Mutex_;

CSystemManager::CSystemManager() {}

CSystemManager::~CSystemManager() {
    delete pInstance_;
    pInstance_ = nullptr;
}

CSystemManager* CSystemManager::get_instance() {
    std::lock_guard<std::mutex> lock(Mutex_);
    if (pInstance_ == nullptr) {
        pInstance_ = new CSystemManager();
    }
    return pInstance_;
}

void CSystemManager::ActivateSystem(ISystem* _System) {
    tSystemContainer.push_back(_System);
    ++s_iSystem_ID;
}

void CSystemManager::DeactivateSystem(DeactivatedSystems system) {
    deactivatedSystems.push_back(system);
}

void CSystemManager::ReturnSystemToActivatedState(DeactivatedSystems system) {
    for (unsigned int i = 0; i < deactivatedSystems.size(); ++i) {
        if (system == deactivatedSystems[i]) {
            deactivatedSystems.erase(deactivatedSystems.begin() + i);
            return;
        }
    }
}

void CSystemManager::Update() {
    bool removedSystemFlag = false;
    for (unsigned int i = 0; i < s_iSystem_ID; ++i) {
        for (unsigned int j = 0; j < deactivatedSystems.size(); ++j) {
            if ((unsigned int)deactivatedSystems[j] == i) {
                removedSystemFlag = true;
                continue;
            }
        }

        if (removedSystemFlag) {
            removedSystemFlag = false;
            continue;
        } else {
            tSystemContainer[i]->Update();
        }
    }
}
} // namespace glvm

namespace glvm {
void CCollisionSystem::Update() {
    // Spatial grid common data.
    const SpatialGrid& spatialGrid = world.spatialGrid;
    assert(
        spatialGrid.width > 0 && spatialGrid.height > 0 && spatialGrid.depth > 0
    );
    const float chunkSize = spatialGrid.grid[0][0][0].size;

    const float chunkHalfWidth = spatialGrid.width * chunkSize * 0.5f;
    const float chunkHalfHeight = spatialGrid.height * chunkSize * 0.5f;
    const float chunkHalfDepth = spatialGrid.depth * chunkSize * 0.5f;

    cachedArchetypesNumber = 0;
    world.searchCacheArchetypes(
        requiredMask,
        cachedArchetypes,
        cachedArchetypesNumber
    );

    const float cameraSpeed = 5.5f * fDelta_Time_;
    // Outer cycle on every archetype.
    for (uint32_t x = 0; x < cachedArchetypesNumber; ++x) {
        Archetype* arch = cachedArchetypes[x];
        view.backtrackingTransforms =
            (transform*)arch->components[ComponentsIndices::TransformComponent];
        view.backtrackingColliders =
            (collider*)arch->components[ComponentsIndices::ColliderComponent];
        view.backtrackingColliderFlags =
            (colliderFlags*)
                arch->components[ComponentsIndices::ColliderFlagsComponent];
        view.backtrackingMeshes =
            (mesh*)arch->components[ComponentsIndices::MeshComponent];

        for (unsigned int i = 0; i < arch->entityCount; ++i) {
            // Count on every entity in current outer archetype.
            uint32_t backtrackingEntityID = arch->entities[i];

            uint8_t groudCollisionTurnOffMask =
                (1u << 0) | (0u << 1) | (1u << 2) | (1u << 3);
            if (view.backtrackingColliderFlags && view.backtrackingColliders
                && view.backtrackingMeshes && view.backtrackingTransforms) {
                view.backtrackingColliderFlags[i].flags =
                    view.backtrackingColliderFlags[i].flags
                    & groudCollisionTurnOffMask;
                view.backtrackingColliders[i].colliders.clear();
                mesh backtrackinEntityMesh = view.backtrackingMeshes[i];
                MeshHandle backtrackingEntityMeshHandle =
                    backtrackinEntityMesh.handle;
                transform* backtrackingTransformComponent =
                    &view.backtrackingTransforms[i];
                Vector<float, 3> backtrackingTransform =
                    backtrackingTransformComponent->position;
                [[maybe_unused]] float backtrackingScale =
                    backtrackingTransformComponent->scale;

                uint64_t moveRequiredMask =
                    (1ul << ComponentsIndices::MoveComponent);
                // Check if outer current archetype has move component.
                if (matches_required_mask(arch->mask, moveRequiredMask)) {
                    view.backtrackingMove =
                        (move*)
                            arch->components[ComponentsIndices::MoveComponent];
                    backtrackingTransform +=
                        Normalize(view.backtrackingMove[i].frameMovement)
                        * cameraSpeed;
                    backtrackingTransform += view.backtrackingMove[i].gravity;
                }

                // Collect entities from grid chunks.
                MeshAxisMaxAbsoluteValues entityChunkBounds =
                    allMeshMaxAbsoluteValues[backtrackingEntityMeshHandle.id];
                std::vector<Vector<float, 3>> entityBoxCornerBoundPoints =
                    computeBoxCornerBoundPoints(
                        entityChunkBounds,
                        backtrackingTransformComponent->position,
                        backtrackingTransformComponent->scale
                    );

                // Result array with collected entities.
                std::vector<uint32_t> collectedEntities;
                // Need only left bottom back corner point and right upper front
                // corner point to obtain all box bounds
                const Vector<float, 3> minEntityPosition =
                    entityBoxCornerBoundPoints[0];
                const Vector<float, 3> maxEntityPosition =
                    entityBoxCornerBoundPoints[1];

                int indexMinX = static_cast<int>(
                    (minEntityPosition[0] + chunkHalfWidth) / chunkSize
                );
                int indexMinY = static_cast<int>(
                    (minEntityPosition[1] + chunkHalfHeight) / chunkSize
                );
                int indexMinZ = static_cast<int>(
                    (minEntityPosition[2] + chunkHalfDepth) / chunkSize
                );

                int indexMaxX = static_cast<int>(
                    (maxEntityPosition[0] + chunkHalfWidth) / chunkSize
                );
                int indexMaxY = static_cast<int>(
                    (maxEntityPosition[1] + chunkHalfHeight) / chunkSize
                );
                int indexMaxZ = static_cast<int>(
                    (maxEntityPosition[2] + chunkHalfDepth) / chunkSize
                );

                // Entity can legitimately leave the fixed-size world grid -
                // clamp to nearest edge cell instead of crashing.
                indexMinX =
                    std::clamp(indexMinX, 0, (int)spatialGrid.width - 1);
                indexMinY =
                    std::clamp(indexMinY, 0, (int)spatialGrid.height - 1);
                indexMinZ =
                    std::clamp(indexMinZ, 0, (int)spatialGrid.depth - 1);
                indexMaxX =
                    std::clamp(indexMaxX, 0, (int)spatialGrid.width - 1);
                indexMaxY =
                    std::clamp(indexMaxY, 0, (int)spatialGrid.height - 1);
                indexMaxZ =
                    std::clamp(indexMaxZ, 0, (int)spatialGrid.depth - 1);

                for (auto i2 = indexMinZ; i2 <= indexMaxZ; ++i2) {
                    for (auto i3 = indexMinY; i3 <= indexMaxY; ++i3) {
                        for (auto i4 = indexMinX; i4 <= indexMaxX; ++i4) {
                            const std::vector<uint32_t>& chunkEntities =
                                spatialGrid.grid[i2][i3][i4].entities;
                            for (uint32_t i5 = 0; i5 < chunkEntities.size();
                                 ++i5) {
                                const uint32_t entity = chunkEntities[i5];
                                if (!isExist(collectedEntities, entity)) {
                                    collectedEntities.push_back(entity);
                                }
                            }
                        }
                    }
                }

                // Inner cycle on every archetype.
                // Count on every entity in current inner archetype.
                for (unsigned int j = 0; j < collectedEntities.size(); ++j) {
                    // Check for same entityID and iteration.
                    uint32_t comparedEntityID = collectedEntities[j];
                    if (backtrackingEntityID == comparedEntityID) {
                        continue;
                    }

                    EntityLocation comparedEntityLocation =
                        world.entityLocations[getId(comparedEntityID)];
                    const uint32_t comparedEntityIndex =
                        comparedEntityLocation.index;

                    MeshHandle comparedEntityMeshHandle;
                    if (comparedEntityLocation.arch == nullptr) {
                        // Entity was removed from the world but a stale
                        // reference survived in the grid, skip it.
                        continue;
                    }
                    if (matches_required_mask(
                            comparedEntityLocation.arch->mask,
                            requiredMask
                        )) {
                        Archetype* arch = comparedEntityLocation.arch;
                        view.comparedTransforms = &(
                            (transform*)arch->components
                                [ComponentsIndices::TransformComponent]
                        )[comparedEntityIndex];
                        view.comparedMeshes = &(
                            (
                                mesh*
                            )arch->components[ComponentsIndices::MeshComponent]
                        )[comparedEntityIndex];
                        comparedEntityMeshHandle = view.comparedMeshes->handle;

                        uint64_t moveRequiredMask =
                            (1ul << ComponentsIndices::MoveComponent);
                        if (matches_required_mask(
                                comparedEntityLocation.arch->mask,
                                moveRequiredMask
                            )) {
                            view.comparedMove = &(
                                (move*)arch
                                    ->components[ComponentsIndices::MoveComponent]
                            )[comparedEntityIndex];
                        }
                    }

                    transform* comparedTransformComponent =
                        view.comparedTransforms;
                    move* comparedMoveComponent = view.comparedMove;

                    Vector<float, 3> comparedTransform =
                        Vector<float, 3>(0.0f, 0.0f, 0.0f);
                    float comparedScale = 0.0f;
                    comparedTransform = comparedTransformComponent->position;
                    comparedScale = comparedTransformComponent->scale;

                    Vector<float, 3> gravityTest {};
                    if (comparedMoveComponent != nullptr) {
                        comparedTransform +=
                            Normalize(comparedMoveComponent->frameMovement)
                            * cameraSpeed;
                        comparedTransform += comparedMoveComponent->gravity;
                        gravityTest = comparedMoveComponent->gravity;
                    }

                    bool boxColliderFlag = false;
                    bool upperActorCheckFlag = false;

                    MeshAxisMaxAbsoluteValues
                        backtrackingMeshAxisMaxAbsoluteValues =
                            allMeshMaxAbsoluteValues[backtrackingEntityMeshHandle
                                                         .id];
                    MeshAxisMaxAbsoluteValues
                        comparedMeshAxisMaxAbsoluteValues = {};
                    if (comparedEntityMeshHandle.id
                        < allMeshMaxAbsoluteValues.size()) {
                        comparedMeshAxisMaxAbsoluteValues =
                            allMeshMaxAbsoluteValues[comparedEntityMeshHandle.id];
                    }

                    boxColliderFlag = BoxCollider(
                        backtrackingTransform,
                        comparedTransform,
                        backtrackingScale,
                        comparedScale,
                        backtrackingMeshAxisMaxAbsoluteValues,
                        comparedMeshAxisMaxAbsoluteValues
                    );

                    if (boxColliderFlag) {
                        upperActorCheckFlag = UpperActorCheck(
                            backtrackingTransform,
                            comparedTransform,
                            backtrackingScale,
                            comparedScale,
                            backtrackingEntityMeshHandle,
                            comparedEntityMeshHandle
                        );
                    }

                    if (upperActorCheckFlag && boxColliderFlag) {
                        uint8_t groudCollisionTurnOnMask =
                            (0u << 0) | (1u << 1) | (0u << 2) | (0u << 3);
                        view.backtrackingColliderFlags[i].flags =
                            view.backtrackingColliderFlags[i].flags
                            | groudCollisionTurnOnMask;
                        view.backtrackingColliders[i].colliders.push_back(
                            comparedEntityID
                        );

                        continue;
                    }

                    if (boxColliderFlag) {
                        uint8_t wallCollisionTurnOnMask =
                            (1u << 0) | (0u << 1) | (0u << 2) | (0u << 3);
                        view.backtrackingColliderFlags[i].flags =
                            view.backtrackingColliderFlags[i].flags
                            | wallCollisionTurnOnMask;
                        view.backtrackingColliders[i].colliders.push_back(
                            comparedEntityID
                        );

                        continue;
                    }
                }
            }
        }
    }
    cachedArchetypesNumber = 0;
}

bool CCollisionSystem::UpperActorCheck(
    Vector<float, 3> backtrackingPosition,
    Vector<float, 3> comparedPosition,
    float backtrackingScale,
    float comparedScale,
    MeshHandle backtrackingMeshHandle,
    MeshHandle comparedMeshHandle
) {
    MeshAxisMaxAbsoluteValues backtrackingMeshAxisMaxAbsoluteValues =
        allMeshMaxAbsoluteValues[backtrackingMeshHandle.id];

    MeshAxisMaxAbsoluteValues comparedMeshAxisMaxAbsoluteValues =
        allMeshMaxAbsoluteValues[comparedMeshHandle.id];

    constexpr float epsilon = 0.15f;
    return backtrackingPosition[1]
        + backtrackingMeshAxisMaxAbsoluteValues.origin_offset_y
            * backtrackingScale
        - backtrackingMeshAxisMaxAbsoluteValues.absolute_y * backtrackingScale
        + epsilon
        > comparedPosition[1]
        + comparedMeshAxisMaxAbsoluteValues.origin_offset_y * comparedScale
        + comparedMeshAxisMaxAbsoluteValues.absolute_y * comparedScale;
}
} // namespace glvm

namespace glvm {
void DamageSystem::Update() {
    cachedAttackableArchetypesNumber = 0;
    world.searchCacheArchetypes(
        attackableRequiredMask,
        archView.cachedAttackableArchetypes,
        cachedAttackableArchetypesNumber
    );

    for (uint32_t x = 0; x < cachedAttackableArchetypesNumber; ++x) {
        Archetype* arch = archView.cachedAttackableArchetypes[x];
        componentsView.attackableAttacks =
            (attack*)arch->components[ComponentsIndices::AttackComponent];
        componentsView.attackableHealth =
            (health*)arch->components[ComponentsIndices::HealthComponent];
        componentsView.attackableFonts =
            (font*)arch->components[ComponentsIndices::FontComponent];

        for (unsigned int i = 0; i < arch->entityCount; ++i) {
            uint64_t entity = arch->entities[i];
            if (&componentsView.attackableHealth[i] != nullptr
                && &componentsView.attackableAttacks[i] != nullptr) {
                health& healthComponent = componentsView.attackableHealth[i];
                attack& attackComponent = componentsView.attackableAttacks[i];

                healthComponent.currentHealth -= attackComponent.damage;
                attackComponent.damage = 0;
                if (healthComponent.currentHealth <= 0) {
                    ArchetypeEntityManager* archEntityManager =
                        ArchetypeEntityManager::get_instance();
                    archEntityManager->removeEntity(entity);
                    world.removeEntity(entity);
                }

                font& fontComponent = componentsView.attackableFonts[i];
                fontComponent.font_string.clear();
                fontComponent.font_string.push_back('4');
                fontComponent.font_string.push_back('0');
                fontComponent.lifeTime = 0;
                fontComponent.removeble = true;
            }
        }
    }

    cachedFontArchetypesNumber = 0;
    world.searchCacheArchetypes(
        fontRequiredMask,
        archView.cachedFontArchetypes,
        cachedFontArchetypesNumber
    );

    for (uint32_t x = 0; x < cachedFontArchetypesNumber; ++x) {
        Archetype* arch = archView.cachedFontArchetypes[x];
        componentsView.fonts =
            (font*)arch->components[ComponentsIndices::FontComponent];

        for (unsigned int i = 0; i < arch->entityCount; ++i) {
            if (componentsView.fonts) {
                font& fontComponent = componentsView.fonts[i];
                if (fontComponent.removeble) {
                    fontComponent.lifeTime += deltaTime;
                }
                if (fontComponent.lifeTime >= 1.5) {}
            }
        }
    }
}
} // namespace glvm

namespace glvm {
void EnemySystem::Update() {
    playerArchetypesNumber = 0;
    world.searchCacheArchetypes(
        playerRequiredMask,
        &archView.playerCachedArchetype,
        playerArchetypesNumber
    );
    componentsView.playerTransforms =
        (transform*)archView.playerCachedArchetype
            ->components[ComponentsIndices::TransformComponent];

    enemyArchetypesNumber = 0;
    world.searchCacheArchetypes(
        enemyRequiredMask,
        &archView.enemyCachedArchetype,
        enemyArchetypesNumber
    );
    componentsView.enemyTransforms =
        (transform*)archView.enemyCachedArchetype
            ->components[ComponentsIndices::TransformComponent];
    componentsView.enemyStates =
        (state*)archView.enemyCachedArchetype
            ->components[ComponentsIndices::StateComponent];
    componentsView.enemies =
        (enemy*)archView.enemyCachedArchetype
            ->components[ComponentsIndices::EnemyComponent];

    projectileArchetypesNumber = 0;
    world.searchCacheArchetypes(
        projectileRequiredMask,
        &archView.projectileArchetype,
        projectileArchetypesNumber
    );

    for (uint32_t j = 0; j < archView.playerCachedArchetype->entityCount; ++j) {
        transform* playerTransformComponent =
            &componentsView.playerTransforms[j];
        for (unsigned int i = 0; i < archView.enemyCachedArchetype->entityCount;
             ++i) {
            transform* enemyTransformComponent =
                &componentsView.enemyTransforms[i];
            state* stateEnemyComponent = &componentsView.enemyStates[i];
            enemy* enemyComponent = &componentsView.enemies[i];

            Vector<float, 3> distance = playerTransformComponent->position
                - enemyTransformComponent->position;
            float cameraSpeed = 5.5f * deltaFrameTime;

            if (projectileCooldown > 0) {
                projectileCooldown -= cameraSpeed;
            }
            if (distance.Length() > enemyComponent->detectRadius
                && stateEnemyComponent->state == States::ATTACK) {
                float deltaLength =
                    distance.Length() - enemyComponent->detectRadius;
                Vector<float, 3> enemyMove =
                    distance * (deltaLength / distance.Length());

                enemyTransformComponent->position += enemyMove;
            }

            if (distance.Length() <= enemyComponent->detectRadius) {
                if (projectileCooldown <= 0) {
                    MeshHandle meshHandle {};
                    const uint32_t sphereMeshHandleIndex = 2;
                    if (meshHandlers.size() > 2) {
                        meshHandle = meshHandlers[sphereMeshHandleIndex];
                    }

                    TextureHandle textureHandle {};
                    const uint32_t grayTextureHandle = 2;
                    if (textureHandlers.size() > 2) {
                        textureHandle = textureHandlers[grayTextureHandle];
                    }

                    const material material = {
                        .diffuseTextureID_ = textureHandle,
                        .specularTextureID_ = textureHandle,
                        .ambient = {0.05f, 0.05f, 0.05f},
                        .shininess = 128.0f * 0.078125f
                    };

                    const damage damage = {
                        .maximumDamage = 40,
                        .minimumDamage = 20,
                        .criticalHitRate = 0,
                        .criticalModifier = 0
                    };

                    ArchetypeEntityManager* archEntityManager =
                        ArchetypeEntityManager::get_instance();
                    uint64_t projectileEntity =
                        archEntityManager->createEntity();
                    world.addEntityToArchetype(
                        projectileEntity,
                        archView.projectileArchetype
                    );
                    EntityLocation projectileLocation =
                        world.entityLocations[getId(projectileEntity)];

                    CreateProjectile(
                        enemyTransformComponent->position,
                        playerTransformComponent->position
                            - enemyTransformComponent->position,
                        meshHandle,
                        material,
                        damage,
                        projectileLocation
                    );

                    soundEngine->CreateSoundSample(
                        "../../../examples/assets/sounds/pistol.wav",
                        5,
                        22050,
                        0.05
                    );
                    projectileCooldown = 5.0;
                }

                stateEnemyComponent->state = States::ATTACK;
            }
        }
    }
}
} // namespace glvm

namespace glvm {
void InventorySystem::Update() {
    if (isInventoryOpened) {
        crosshairArchetypesNumber = 0;
        world.searchCacheArchetypes(
            crosshairRequiredMask,
            &archView.crosshairCachedArchetype,
            crosshairArchetypesNumber
        );
        componentsView.crosshairTransformsView =
            (transform*)archView.crosshairCachedArchetype
                ->components[ComponentsIndices::TransformComponent];

        inventoryArchetypesNumber = 0;
        world.searchCacheArchetypes(
            inventoryRequiredMask,
            &archView.inventoryCachedArchetype,
            inventoryArchetypesNumber
        );

        componentsView.inventoryTransformsView =
            (transform*)archView.inventoryCachedArchetype
                ->components[ComponentsIndices::TransformComponent];
        componentsView.inventoryView =
            (inventory*)archView.inventoryCachedArchetype
                ->components[ComponentsIndices::InventoryComponent];
        componentsView.inventoryMeshesView =
            (mesh*)archView.inventoryCachedArchetype
                ->components[ComponentsIndices::MeshComponent];

        if (componentsView.crosshairTransformsView
            && componentsView.inventoryTransformsView
            && componentsView.inventoryView
            && componentsView.inventoryMeshesView) {
            transform* crosshairTransformComponent =
                &componentsView.crosshairTransformsView[0];

            transform* inventoryTransformComponent =
                &componentsView.inventoryTransformsView[0];
            inventory* inventoryComponent = &componentsView.inventoryView[0];
            mesh* inventoryMeshComponent =
                &componentsView.inventoryMeshesView[0];

            const float inventorySlotScale = inventoryMeshComponent->gltf
                ? inventoryComponent->slotScale * 2.0f
                : inventoryComponent->slotScale;
            const float inventorySlotHalfScale = inventoryMeshComponent->gltf
                ? inventoryComponent->slotScale
                : inventoryComponent->slotScale * 0.5f;

            // Take an item from inventory.
            if (*isItemDraged < 0 && isLeftMouseButtonPressed
                && *isLeftMouseButtonReleased) {
                if (checkCrosshairInventoryIntersection(
                        crosshairTransformComponent,
                        inventoryTransformComponent,
                        inventoryComponent,
                        inventorySlotScale,
                        inventorySlotHalfScale
                    )) {
                    point2D<int> intersectionSlot =
                        determineActualIntersectionSlot(
                            crosshairTransformComponent,
                            inventoryTransformComponent,
                            inventorySlotScale,
                            inventorySlotHalfScale
                        );
                    const unsigned int row = intersectionSlot.y;
                    const unsigned int column = intersectionSlot.x;
                    const unsigned int entity =
                        inventoryComponent->slots[row][column];
                    // Check slot is not empty and hold an item.
                    if (entity != UINT_MAX && entity >= 0) {
                        EntityLocation itemLocation =
                            world.entityLocations[getId(entity)];
                        ItemArchetype* itemArch =
                            static_cast<ItemArchetype*>(itemLocation.arch);
                        const uint32_t itemIndex = itemLocation.index;
                        item* itemComponent = &itemArch->items[itemIndex];

                        if (itemComponent != nullptr) {
                            for (unsigned int i = 0;
                                 i < itemComponent->occupiedSlots.size();
                                 ++i) {
                                unsigned int row_index =
                                    itemComponent->occupiedSlots[i]
                                    / inventoryComponent->row;
                                unsigned int col_index =
                                    itemComponent->occupiedSlots[i]
                                    % inventoryComponent->col;
                                // Need to free all slots that hold an item.
                                inventoryComponent->slots[row_index][col_index] =
                                    UINT_MAX;
                            }
                        }
                        // Set currently dragged item entity.
                        *isItemDraged = entity;
                    }
                }
                *isLeftMouseButtonReleased = false;
            }

            if (*isItemDraged >= 0) {
                if (checkCrosshairInventoryIntersection(
                        crosshairTransformComponent,
                        inventoryTransformComponent,
                        inventoryComponent,
                        inventorySlotScale,
                        inventorySlotHalfScale
                    )) {
                    [[maybe_unused]] point2D<int> intersectionSlot =
                        determineActualIntersectionSlot(
                            crosshairTransformComponent,
                            inventoryTransformComponent,
                            inventorySlotScale,
                            inventorySlotHalfScale
                        );

                    EntityLocation itemLocation =
                        world.entityLocations[getId(*isItemDraged)];
                    ItemArchetype* itemArch =
                        static_cast<ItemArchetype*>(itemLocation.arch);
                    const uint32_t itemIndex = itemLocation.index;
                    item* itemComponent = &itemArch->items[itemIndex];

                    std::vector<unsigned int> potentialOccupiedSlots;
                    int isSwapable = 0;
                    isSwapable = determineSwappableStatusAndSlots(
                        itemComponent,
                        inventoryTransformComponent,
                        potentialOccupiedSlots,
                        crosshairTransformComponent,
                        intersectionSlot,
                        inventoryComponent,
                        inventorySlotScale
                    );

                    inventoryComponent->highlightedSlots =
                        potentialOccupiedSlots;
                    inventoryComponent->isAvailableHighlightedSlots =
                        isSwapable == -1 || isSwapable >= 0;
                } else {
                    inventoryComponent->highlightedSlots.clear();
                }
            }

            // Item drop to inventory, swapped or we just can't place.
            if (*isItemDraged >= 0 && isLeftMouseButtonPressed
                && *isLeftMouseButtonReleased) {
                int isSwapable = 0;
                if (checkCrosshairInventoryIntersection(
                        crosshairTransformComponent,
                        inventoryTransformComponent,
                        inventoryComponent,
                        inventorySlotScale,
                        inventorySlotHalfScale
                    )) {
                    [[maybe_unused]] point2D<int> intersectionSlot =
                        determineActualIntersectionSlot(
                            crosshairTransformComponent,
                            inventoryTransformComponent,
                            inventorySlotScale,
                            inventorySlotHalfScale
                        );

                    EntityLocation itemLocation =
                        world.entityLocations[getId(*isItemDraged)];
                    ItemArchetype* itemArch =
                        static_cast<ItemArchetype*>(itemLocation.arch);
                    const uint32_t itemIndex = itemLocation.index;
                    item* itemComponent = &itemArch->items[itemIndex];

                    std::vector<unsigned int> potentialOccupiedSlots;
                    isSwapable = determineSwappableStatusAndSlots(
                        itemComponent,
                        inventoryTransformComponent,
                        potentialOccupiedSlots,
                        crosshairTransformComponent,
                        intersectionSlot,
                        inventoryComponent,
                        inventorySlotScale
                    );

                    const int itemWidth = itemComponent->itemSlotType.width;
                    const int itemHeight = itemComponent->itemSlotType.height;

                    // Default value. Just drop item to all empty slots.
                    if (isSwapable == -1) {
                        itemComponent->occupiedSlots = potentialOccupiedSlots;
                        fillInventorySlots(
                            itemComponent,
                            itemWidth,
                            itemHeight,
                            inventoryComponent,
                            *isItemDraged
                        );
                        // Swap one item that we dragging to another one in
                        // inventory.
                    } else if (isSwapable > 0) {
                        EntityLocation itemLocation =
                            world.entityLocations[getId(isSwapable)];
                        ItemArchetype* itemArch =
                            static_cast<ItemArchetype*>(itemLocation.arch);
                        const uint32_t itemIndex = itemLocation.index;
                        item* swapedItemComponent = &itemArch->items[itemIndex];

                        itemComponent->occupiedSlots = potentialOccupiedSlots;
                        fillInventorySlots(
                            swapedItemComponent,
                            swapedItemComponent->itemSlotType.width,
                            swapedItemComponent->itemSlotType.height,
                            inventoryComponent,
                            UINT_MAX
                        );

                        swapedItemComponent->occupiedSlots.clear();
                        fillInventorySlots(
                            itemComponent,
                            itemWidth,
                            itemHeight,
                            inventoryComponent,
                            *isItemDraged
                        );
                    }

                    if (isSwapable == -1) {
                        *isLeftMouseButtonReleased = false;
                        *isItemDraged = -1;
                        // Already have 2 or more items in potential inventory
                        // slots.
                    } else if (isSwapable == -2) {
                        *isLeftMouseButtonReleased = false;
                    } else {
                        *isLeftMouseButtonReleased = false;
                        *isItemDraged = isSwapable;
                    }
                    // Item drop to the ground.
                } else {
                    EntityLocation itemLocation =
                        world.entityLocations[getId(*isItemDraged)];
                    ItemArchetype* itemArch =
                        static_cast<ItemArchetype*>(itemLocation.arch);
                    const uint32_t itemIndex = itemLocation.index;
                    itemArch->rigidBodies[itemIndex] = {.fMass_ = 2.0f};
                    transform* itemTransform = &itemArch->transforms[itemIndex];
                    item* item = &itemArch->items[itemIndex];
                    item->isActor = true;
                    // Remove this cringe.
                    const uint32_t player = 0;
                    EntityLocation playerLocation =
                        world.entityLocations[getId(player)];
                    PlayerArchetype* playerArch =
                        static_cast<PlayerArchetype*>(playerLocation.arch);
                    const uint32_t playerIndex = playerLocation.index;
                    transform* playerTransform =
                        &playerArch->transforms[playerIndex];
                    itemTransform->position = playerTransform->position;
                    Vector<float, 3> normalizedForward =
                        Normalize(playerTransform->forward);
                    itemTransform->position[0] += normalizedForward[0] * 2.5f;
                    itemTransform->position[1] += normalizedForward[1] * 2.5f;
                    itemTransform->position[2] += normalizedForward[2] * 2.5f;
                    itemTransform->scale = 0.05f;

                    *isItemDraged = -1;
                    *isLeftMouseButtonReleased = false;
                }
            }
        }
    }
}

int InventorySystem::determineSwappableStatusAndSlots(
    item* itemComponent,
    transform* inventoryTransformComponent,
    std::vector<unsigned int>& potentialOccupiedSlots,
    transform* crosshairTransformComponent,
    point2D<int> intersectionSlot,
    inventory* inventoryComponent,
    const float inventorySlotScale
) {
    if (itemComponent != nullptr) {
        const int itemWidth = itemComponent->itemSlotType.width;
        const int itemHeight = itemComponent->itemSlotType.height;

        const int row = intersectionSlot.y;
        const int column = intersectionSlot.x;

        // Find left-upper pivot slot inventory.
        int rowBasicOffset = 0;
        int columnBasicOffset = 0;

        // Set as pivot point slot in left upper corner.
        // Need to calculate offset for row and column
        // to change it from center. And need to It is
        // necessary to take into account the offset
        // relative to the center for additional correction.
        columnBasicOffset = calculateBasicOffset(
            itemWidth,
            inventoryTransformComponent->position[0],
            crosshairTransformComponent->position[0],
            column,
            inventorySlotScale
        );
        rowBasicOffset = calculateBasicOffset(
            itemHeight,
            inventoryTransformComponent->position[1],
            crosshairTransformComponent->position[1],
            row,
            inventorySlotScale * aspectRate
        );

        int pivotRow = row - rowBasicOffset;
        int pivotColumn = column - columnBasicOffset;

        clamp<int>(
            0,
            pivotRow,
            static_cast<int>(inventoryComponent->row) - itemHeight
        );
        clamp<int>(
            0,
            pivotColumn,
            static_cast<int>(inventoryComponent->col) - itemWidth
        );

        return determineSwappableField(
            itemComponent,
            itemWidth,
            itemHeight,
            pivotRow,
            pivotColumn,
            inventoryComponent,
            potentialOccupiedSlots
        );
    } else {
        // Return -3 as error code means itemComponent is nullptr.
        return -3;
    }
}

void InventorySystem::fillInventorySlots(
    item* itemComponent,
    const int itemWidth,
    const int itemHeight,
    inventory* inventoryComponent,
    const int fillValue
) {
    for (int i = 0; i < itemHeight; ++i) {
        for (int j = 0; j < itemWidth; ++j) {
            const unsigned int slotsRow =
                itemComponent->occupiedSlots[i * itemWidth + j]
                / inventoryComponent->col;
            const unsigned int slotsColumn =
                itemComponent->occupiedSlots[i * itemWidth + j]
                % inventoryComponent->col;

            inventoryComponent->slots[slotsRow][slotsColumn] = fillValue;
        }
    }
}

int InventorySystem::determineSwappableField(
    item* itemComponent,
    const int itemWidth,
    const int itemHeight,
    int pivotRow,
    int pivotColumn,
    inventory* inventoryComponent,
    std::vector<unsigned int>& potentialOccupiedSlots
) {
    itemComponent->occupiedSlots.clear();
    // -1: default value. -2: found two entities in potential slots. Any other
    // value: swappable.
    int isSwapable = -1;
    for (int i = 0; i < itemHeight; ++i) {
        for (int j = 0; j < itemWidth; ++j) {
            const unsigned int finalRow = pivotRow + i;
            const unsigned int finalColumn = pivotColumn + j;
            if (isSwapable == -1
                && inventoryComponent->slots[finalRow][finalColumn]
                    != UINT_MAX) {
                isSwapable = inventoryComponent->slots[finalRow][finalColumn];
            } else if (
                isSwapable > 0
                && inventoryComponent->slots[finalRow][finalColumn] != UINT_MAX
                && (int)inventoryComponent->slots[finalRow][finalColumn]
                    != isSwapable
            ) {
                isSwapable = -2;
            }

            potentialOccupiedSlots.push_back(
                finalRow * inventoryComponent->col + finalColumn
            );
        }
    }

    return isSwapable;
}

int InventorySystem::calculateBasicOffset(
    const int itemAxisSize,
    const float axisValue,
    const float crosshairAxisPosition,
    const int axisSlotIndex,
    const float inventorySlotScale
) {
    if (itemAxisSize % 2 == 0) {
        const float slotCenterX =
            axisValue + static_cast<float>(axisSlotIndex) * inventorySlotScale;
        if (slotCenterX > crosshairAxisPosition) {
            return itemAxisSize / 2;
        }
        return itemAxisSize / 2 - 1;
    }
    return itemAxisSize / 2;
}

bool InventorySystem::checkCrosshairInventoryIntersection(
    transform* crosshairTransformComponent,
    transform* inventoryTransformComponent,
    inventory* inventoryComponent,
    const float inventorySlotScale,
    const float inventorySlotHalfScale
) {
    return crosshairTransformComponent->position[0]
        > inventoryTransformComponent->position[0] - inventorySlotHalfScale
        && crosshairTransformComponent->position[0]
        < inventoryTransformComponent->position[0] - inventorySlotHalfScale
            + inventorySlotScale * inventoryComponent->col
        && crosshairTransformComponent->position[1]
        > inventoryTransformComponent->position[1]
            - inventorySlotHalfScale * aspectRate
        && crosshairTransformComponent->position[1]
        < inventoryTransformComponent->position[1]
            - inventorySlotHalfScale * aspectRate
            + inventorySlotScale * inventoryComponent->row * aspectRate;
}

point2D<int> InventorySystem::determineActualIntersectionSlot(
    transform* crosshairTransformComponent,
    transform* inventoryTransformComponent,
    const float inventorySlotScale,
    const float inventorySlotHalfScale
) {
    float x_delta = crosshairTransformComponent->position[0]
        - inventoryTransformComponent->position[0] + inventorySlotHalfScale;
    float y_delta = crosshairTransformComponent->position[1]
        - inventoryTransformComponent->position[1]
        + inventorySlotHalfScale * aspectRate;

    return point2D<int> {
        (int)(x_delta / inventorySlotScale),
        (int)(y_delta / (inventorySlotScale * aspectRate))
    };
}
} // namespace glvm

namespace glvm {
// This method is trying to search for suitable slots for the given specific
// type item. It returns true if it finds them and false otherwise.
bool ItemSystem::putItem2x2(
    inventory* inventoryComponent,
    unsigned int itemEntity
) {
    bool isSlotFound = false;
    unsigned int row = inventoryComponent->row;
    unsigned int col = inventoryComponent->col;

    EntityLocation itemLocation = world.entityLocations[getId(itemEntity)];
    ItemArchetype* itemArch = static_cast<ItemArchetype*>(itemLocation.arch);
    const uint32_t itemIndex = itemLocation.index;
    item* itemComponent = &itemArch->items[itemIndex];

    unsigned int item_width = itemComponent->itemSlotType.width;
    unsigned int item_height = itemComponent->itemSlotType.height;

    std::cout << "item width: " << item_width << std::endl;
    std::cout << "item height: " << item_height << std::endl;
    for (unsigned int i = 0; i < row - item_height + 1; ++i) {
        for (unsigned int j = 0; j < col - item_width + 1; ++j) {
            std::vector<unsigned int> maybeAvailabeSlots;
            std::vector<unsigned int> indicesOfMaybeAvailableSlots;
            for (unsigned int m = i; m < i + item_height; ++m) {
                for (unsigned int n = j; n < j + item_width; ++n) {
                    maybeAvailabeSlots.push_back(
                        inventoryComponent->slots[m][n]
                    );
                    indicesOfMaybeAvailableSlots.push_back(m * col + n);
                }
            }

            unsigned int isAllSlotsAvailable = 0;
            for (unsigned int v = 0; v < maybeAvailabeSlots.size(); ++v) {
                if (maybeAvailabeSlots[v] == UINT_MAX) {
                    ++isAllSlotsAvailable;
                } else {
                    --isAllSlotsAvailable;
                }
            }
            if (maybeAvailabeSlots.size() == isAllSlotsAvailable) {
                for (unsigned int w = 0; w < maybeAvailabeSlots.size(); ++w) {
                    unsigned int row_index =
                        indicesOfMaybeAvailableSlots[w] / row;
                    unsigned int col_index =
                        indicesOfMaybeAvailableSlots[w] % col;
                    inventoryComponent->slots[row_index][col_index] =
                        itemEntity;
                    itemComponent->occupiedSlots.push_back(
                        indicesOfMaybeAvailableSlots[w]
                    );
                }

                isSlotFound = true;
                return isSlotFound;
            }
        }
    }

    return isSlotFound;
}

void ItemSystem::Update() {
    if (!isInventoryOpened) {
        inventoryArchetypesNumber = 0;
        world.searchCacheArchetypes(
            inventoryRequiredMask,
            &archView.inventoryCachedArchetype,
            inventoryArchetypesNumber
        );
        componentsView.inventoriesView =
            (inventory*)archView.inventoryCachedArchetype
                ->components[ComponentsIndices::InventoryComponent];

        itemArchetypesNumber = 0;
        world.searchCacheArchetypes(
            itemRequiredMask,
            &archView.itemArchetype,
            itemArchetypesNumber
        );
        componentsView.itemsView =
            (item*)archView.itemArchetype
                ->components[ComponentsIndices::ItemComponent];
        componentsView.itemCollidersView =
            (collider*)archView.itemArchetype
                ->components[ComponentsIndices::ColliderComponent];

        for (unsigned int m = 0;
             m < archView.inventoryCachedArchetype->entityCount;
             ++m) {
            inventory* inventoryComponent = &componentsView.inventoriesView[m];

            for (unsigned int i = 0; i < archView.itemArchetype->entityCount;
                 ++i) {
                unsigned int itemEntity = archView.itemArchetype->entities[i];
                collider* itemColliderComponent =
                    &componentsView.itemCollidersView[i];

                for (unsigned int j = 0;
                     j < itemColliderComponent->colliders.size();
                     ++j) {
                    if (itemColliderComponent->colliders[j]
                            == inventoryComponent->entityOwner
                        && componentsView.itemsView[i].isActor) {
                        if (putItem2x2(inventoryComponent, itemEntity)) {
                            componentsView.itemsView[i].isActor = false;
                        } else {
                            std::cout
                                << "No suitable slots for that item in inventory"
                                << std::endl;
                        }
                    }
                }
            }
        }
    }

    if (isInventoryOpened) {
        crosshairArchetypesNumber = 0;
        world.searchCacheArchetypes(
            crosshairRequiredMask,
            &archView.crosshairArchetype,
            crosshairArchetypesNumber
        );
        componentsView.crosshairTransforms =
            (transform*)archView.crosshairArchetype
                ->components[ComponentsIndices::TransformComponent];

        itemArchetypesNumber = 0;
        world.searchCacheArchetypes(
            itemRequiredMask,
            &archView.itemArchetype,
            itemArchetypesNumber
        );
        componentsView.itemTransformsView =
            (transform*)archView.itemArchetype
                ->components[ComponentsIndices::TransformComponent];

        transform* crosshairTransformComponent =
            &componentsView.crosshairTransforms[0];
        for (unsigned int i = 0; i < archView.itemArchetype->entityCount; ++i) {
            uint32_t entityItemContaining = archView.itemArchetype->entities[i];
            transform* itemTransformComponent =
                &componentsView.itemTransformsView[i];
            if (*draggedItemEntity >= 0
                && *draggedItemEntity == (int)entityItemContaining) {
                // Set crosshair position to dragged items.
                itemTransformComponent->position =
                    crosshairTransformComponent->position;
            }
        }
    }
}
} // namespace glvm

namespace glvm {
CMovementSystem::CMovementSystem(CStack& inputStack) : inputStack(inputStack) {}

void CMovementSystem::Update() {
    world.searchCacheArchetypes(
        playerRequiredMask,
        &archView.playerCachedArchetype,
        playerArchetypesNumber
    );
    componentsView.playerMoves =
        (move*)archView.playerCachedArchetype
            ->components[ComponentsIndices::MoveComponent];
    componentsView.playerViews =
        (beholder*)archView.playerCachedArchetype
            ->components[ComponentsIndices::ViewComponent];
    componentsView.playerColliderFlags =
        (colliderFlags*)archView.playerCachedArchetype
            ->components[ComponentsIndices::ColliderFlagsComponent];
    componentsView.playerRigidBody =
        (RigidBody*)archView.playerCachedArchetype
            ->components[ComponentsIndices::RigidBodyComponent];

    const float cameraSpeed = 3.0f * deltaFrameTime;
    for (unsigned int i = 0; i < archView.playerCachedArchetype->entityCount;
         ++i) {
        const uint64_t entity = archView.playerCachedArchetype->entities[i];
        EntityLocation& entityLocation = world.entityLocations[getId(entity)];
        beholder* playerView = &componentsView.playerViews[i];
        move* playerMove = &componentsView.playerMoves[i];
        colliderFlags* playerColliderFlags =
            &componentsView.playerColliderFlags[i];
        RigidBody* playerRigidBody = &componentsView.playerRigidBody[i];
        for (int n = 0; n < 6; ++n) {
            Vector<float, 3> right;
            Vector<float, 3> forward;
            switch (inputStack[n]) {
                case EEvents::eMOVE_LEFT:
                    right = CalculateVectorRL(*playerView);
                    playerMove->frameMovement -= right * cameraSpeed;
                    entityLocation.isDirty = true;
                    break;
                case EEvents::eMOVE_RIGHT:
                    right = CalculateVectorRL(*playerView);
                    playerMove->frameMovement += right * cameraSpeed;
                    entityLocation.isDirty = true;
                    break;
                case EEvents::eMOVE_BACKWARD:
                    forward = CalculateVectorFB(*playerView, g_eEvent);
                    playerMove->frameMovement -= forward * cameraSpeed;
                    entityLocation.isDirty = true;
                    break;
                case EEvents::eMOVE_FORWARD:
                    forward = CalculateVectorFB(*playerView, g_eEvent);
                    playerMove->frameMovement += forward * cameraSpeed;
                    entityLocation.isDirty = true;
                    break;
                case EEvents::eJUMP: {
                    entityLocation.isDirty = true;
                    uint8_t isGroudCollisionMask =
                        (0u << 0) | (1u << 1) | (0u << 2) | (0u << 3);
                    if (playerColliderFlags->flags & isGroudCollisionMask) {
                        playerRigidBody->jumpAccumulator = 1.5f;
                    }
                } break;
                default:
                    break;
            }
        }
    }

    rigidBodyContainedArchetypesNumber = 0;
    world.searchCacheArchetypes(
        rigidBodyRequiredMask,
        archView.rigidBodyContainedArchetypesCache,
        rigidBodyContainedArchetypesNumber
    );

    for (uint32_t i0 = 0; i0 < rigidBodyContainedArchetypesNumber; ++i0) {
        Archetype* currentArch = archView.rigidBodyContainedArchetypesCache[i0];
        componentsView.transforms =
            (transform*)
                currentArch->components[ComponentsIndices::TransformComponent];
        componentsView.rigidBodies =
            (RigidBody*)
                currentArch->components[ComponentsIndices::RigidBodyComponent];
        componentsView.moves =
            (move*)currentArch->components[ComponentsIndices::MoveComponent];
        componentsView.items =
            (item*)currentArch->components[ComponentsIndices::ItemComponent];

        for (uint32_t i1 = 0; i1 < currentArch->entityCount; ++i1) {
            const uint64_t entity = currentArch->entities[i1];
            EntityLocation& entityLocation =
                world.entityLocations[getId(entity)];
            entityLocation.isDirty = true;

            if (componentsView.items && !componentsView.items[i1].isActor) {
                continue;
            }

            transform* rTransform_Component = &componentsView.transforms[i1];
            RigidBody* rigidBodyComponennt = &componentsView.rigidBodies[i1];
            move* moveComponent = &componentsView.moves[i1];
            rTransform_Component->gravityAccumulator += deltaFrameTime;
            float gravity = 9.8f * rTransform_Component->gravityAccumulator
                * rigidBodyComponennt->fMass_ * 0.0005;
            if (gravity > 0.2f) {
                gravity = 0.2;
            }

            moveComponent->gravity[1] -= gravity;
        }
    }
}

Vector<float, 3> CMovementSystem::CalculateVectorRL(beholder& beholder) {
    Vector<float, 3> normalizedVector =
        Normalize(Cross(beholder.forward, Vector<float, 3> {0.0f, -1.0f, 0.0}));
    return normalizedVector;
}

Vector<float, 3> CMovementSystem::CalculateVectorFB(
    beholder& beholder,
    [[maybe_unused]] CEvent& event
) {
    Vector<float, 3> forward(0.0f);
    current_X = (float)g_eEvent.mousePointerPosition.offset_X;
    float delta_x = current_X - prev_X;
    const Vector<float, 3> rotateAxis = {0.0, -1.0, 0.0};
    float rotationAngle = delta_x;
    constexpr float angleScale = 0.1f;
    rotationAngle = Radians(rotationAngle * angleScale);
    // Quaternions need division by 2.
    constexpr float quatAngleCorrection = 0.5f;
    const float sinRotationAngle = sinf(rotationAngle * quatAngleCorrection);
    Quaternion rotationQuat = Quaternion(
        cosf(rotationAngle * quatAngleCorrection),
        sinRotationAngle * rotateAxis[0],
        sinRotationAngle * rotateAxis[1],
        sinRotationAngle * rotateAxis[2]
    );
    const Quaternion appliedRotationQuat = (rotationQuat
                                            * Quaternion(
                                                0.0f,
                                                beholder.forward[0],
                                                beholder.forward[1],
                                                beholder.forward[2]
                                            ))
        * conjugate(rotationQuat);

    forward[0] = appliedRotationQuat.x;
    forward[1] = 0.0f;
    forward[2] = appliedRotationQuat.z;
    prev_X = (float)g_eEvent.mousePointerPosition.offset_X;
    forward = Normalize(forward);
    return forward;
}
} // namespace glvm

namespace glvm {
namespace {
bool aabbOverlap(
    const Vector<float, 3>& aPosition,
    const MeshAxisMaxAbsoluteValues& aBounds,
    float aScale,
    const Vector<float, 3>& bPosition,
    const MeshAxisMaxAbsoluteValues& bBounds,
    float bScale
) {
    return aPosition[0] + aBounds.origin_offset_x * aScale
            + aBounds.absolute_x * aScale
        > bPosition[0] + bBounds.origin_offset_x * bScale
            - bBounds.absolute_x * bScale
        && aPosition[0] + aBounds.origin_offset_x * aScale
            - aBounds.absolute_x * aScale
        < bPosition[0] + bBounds.origin_offset_x * bScale
            + bBounds.absolute_x * bScale
        && aPosition[1] + aBounds.origin_offset_y * aScale
            + aBounds.absolute_y * aScale
        > bPosition[1] + bBounds.origin_offset_y * bScale
            - bBounds.absolute_y * bScale
        && aPosition[1] + aBounds.origin_offset_y * aScale
            - aBounds.absolute_y * aScale
        < bPosition[1] + bBounds.origin_offset_y * bScale
            + bBounds.absolute_y * bScale
        && aPosition[2] + aBounds.origin_offset_z * aScale
            + aBounds.absolute_z * aScale
        > bPosition[2] + bBounds.origin_offset_z * bScale
            - bBounds.absolute_z * bScale
        && aPosition[2] + aBounds.origin_offset_z * aScale
            - aBounds.absolute_z * aScale
        < bPosition[2] + bBounds.origin_offset_z * bScale
            + bBounds.absolute_z * bScale;
}

bool isAbove(
    const Vector<float, 3>& aPosition,
    const MeshAxisMaxAbsoluteValues& aBounds,
    float aScale,
    const Vector<float, 3>& bPosition,
    const MeshAxisMaxAbsoluteValues& bBounds,
    float bScale
) {
    constexpr float epsilon = 0.15f;
    return aPosition[1] + aBounds.origin_offset_y * aScale
        - aBounds.absolute_y * aScale + epsilon
        > bPosition[1] + bBounds.origin_offset_y * bScale
        + bBounds.absolute_y * bScale;
}
} // namespace

// This update searching for referring to colliders entities and check their
// transform components for collision, and if collision detected check if
// backtracking entity had gravity component for call Gravity function.
void CPhysicsSystem::Update() {
    cachedArchetypesNumber = 0;
    world.searchCacheArchetypes(
        requiredMask,
        archView.cachedArchetypes,
        cachedArchetypesNumber
    );

    for (uint32_t x = 0; x < cachedArchetypesNumber; ++x) {
        Archetype* arch = archView.cachedArchetypes[x];

        componentsView.transformsView =
            (transform*)archView.cachedArchetypes[x]
                ->components[ComponentsIndices::TransformComponent];
        componentsView.movesView =
            (move*)archView.cachedArchetypes[x]
                ->components[ComponentsIndices::MoveComponent];
        componentsView.rigidBodiesView =
            (RigidBody*)archView.cachedArchetypes[x]
                ->components[ComponentsIndices::RigidBodyComponent];
        componentsView.colliderFlagsView =
            (colliderFlags*)archView.cachedArchetypes[x]
                ->components[ComponentsIndices::ColliderFlagsComponent];
        componentsView.collidersView =
            (collider*)archView.cachedArchetypes[x]
                ->components[ComponentsIndices::ColliderComponent];
        componentsView.meshesView =
            (mesh*)archView.cachedArchetypes[x]
                ->components[ComponentsIndices::MeshComponent];

        float deltaTime = 5.5f * fDelta_Time_;
        for (unsigned int i = 0; i < arch->entityCount; ++i) {
            if (componentsView.transformsView
                && componentsView.colliderFlagsView && componentsView.movesView
                && componentsView.rigidBodiesView) {
                transform& transformComponent =
                    componentsView.transformsView[i];
                move& move = componentsView.movesView[i];
                colliderFlags& colliderFlags =
                    componentsView.colliderFlagsView[i];
                uint8_t isGroudCollisionMask =
                    (0u << 0) | (1u << 1) | (0u << 2) | (0u << 3);
                if (colliderFlags.flags & isGroudCollisionMask) {
                    move.gravity = 0;
                    transformComponent.gravityAccumulator = 0.0f;
                }
                uint8_t isWallCollisionMask =
                    (1u << 0) | (0u << 1) | (0u << 2) | (0u << 3);
                if (colliderFlags.flags & isWallCollisionMask) {
                    // Wall-slide: zero only the frameMovement axis blocked by
                    // a collider, keep the tangential component so the player
                    // slides along the wall instead of sticking to it.
                    collider* colliders = componentsView.collidersView;
                    mesh* meshes = componentsView.meshesView;
                    if (colliders && meshes
                        && colliders[i].colliders.size() > 0) {
                        const MeshAxisMaxAbsoluteValues playerBounds =
                            allMeshMaxAbsoluteValues[meshes[i].handle.id];
                        const Vector<float, 3> playerPosition =
                            transformComponent.position;
                        for (uint32_t c = 0; c < colliders[i].colliders.size();
                             ++c) {
                            const uint32_t collidedEntity =
                                colliders[i].colliders[c];
                            EntityLocation& collidedLocation =
                                world.entityLocations[getId(collidedEntity)];
                            Archetype* collidedArch = collidedLocation.arch;
                            if (collidedArch == nullptr) {
                                // Entity was removed this frame (e.g. by
                                // DamageSystem) after collision detection.
                                continue;
                            }
                            const uint32_t collidedIndex =
                                collidedLocation.index;
                            transform* collidedTransform =
                                (transform*)collidedArch->components
                                    [ComponentsIndices::TransformComponent];
                            mesh* collidedMesh =
                                (mesh*)collidedArch->components
                                    [ComponentsIndices::MeshComponent];
                            if (!collidedTransform || !collidedMesh) {
                                continue;
                            }
                            collidedTransform += collidedIndex;
                            collidedMesh += collidedIndex;
                            const MeshAxisMaxAbsoluteValues collidedBounds =
                                allMeshMaxAbsoluteValues[collidedMesh->handle.id];
                            const Vector<float, 3> collidedPosition =
                                collidedTransform->position;
                            // Ground (player standing above) is handled by
                            // gravity, only resolve wall-like colliders.
                            if (isAbove(
                                    playerPosition,
                                    playerBounds,
                                    transformComponent.scale,
                                    collidedPosition,
                                    collidedBounds,
                                    collidedTransform->scale
                                )) {
                                continue;
                            }
                            for (int axis = 0; axis < 3; ++axis) {
                                Vector<float, 3> candidate = playerPosition;
                                candidate[axis] += move.frameMovement[axis];
                                bool hit = aabbOverlap(
                                    candidate,
                                    playerBounds,
                                    transformComponent.scale,
                                    collidedPosition,
                                    collidedBounds,
                                    collidedTransform->scale
                                );
                                if (hit) {
                                    move.frameMovement[axis] = 0.0f;
                                }
                            }
                        }
                    } else {
                        move.frameMovement = 0;
                    }
                    uint8_t wallCollisionTurnOffMask =
                        (0u << 0) | (1u << 1) | (1u << 2) | (1u << 3);
                    colliderFlags.flags &= wallCollisionTurnOffMask;
                }
                transformComponent.position += move.frameMovement;
                transformComponent.position += move.gravity;
                move.gravity = 0.0f;
                move.frameMovement = 0.0f;
                RigidBody& rigidBody = componentsView.rigidBodiesView[i];
                if (rigidBody.jumpAccumulator > 0.0f) {
                    rigidBody.jumpAccumulator -= deltaTime;
                    Vector<float, 3> jump =
                        Vector<float, 3> {0.0f, 5.0f, 0.0f} * deltaTime;
                    transformComponent.position += jump;
                }
            }
        }
    }
}
} // namespace glvm

namespace glvm {
CProjectileSystem::CProjectileSystem(CStack& inputStack) :
    inputStack(inputStack) {}

void CProjectileSystem::Update() {
    float cameraSpeed = 5.5f * deltaFrameTime;

    playerArchetypesNumber = 0;
    world.searchCacheArchetypes(
        playerRequiredMask,
        &archView.playerCachedArchetype,
        playerArchetypesNumber
    );
    componentsView.playerTransforms =
        (transform*)archView.playerCachedArchetype
            ->components[ComponentsIndices::TransformComponent];
    componentsView.playerViews =
        (beholder*)archView.playerCachedArchetype
            ->components[ComponentsIndices::ViewComponent];

    projectileArchetypesNumber = 0;
    world.searchCacheArchetypes(
        projectileRequiredMask,
        &archView.projectileArchetype,
        projectileArchetypesNumber
    );

    if (projectileCooldown > 0) {
        projectileCooldown -= cameraSpeed;
    }

    // Iterate on every player and create projectile if "LMB pressed" event
    // found.
    for (unsigned int i = 0; i < archView.playerCachedArchetype->entityCount;
         ++i) {
        beholder* playerView = &componentsView.playerViews[i];
        transform* playerTransform = &componentsView.playerTransforms[i];
        const uint32_t maxEventNumber = 6;
        for (uint32_t n = 0; n < maxEventNumber; ++n) {
            if (!isInventoryOpened
                && inputStack.SearchElement(EEvents::eMOUSE_LEFT_BUTTON)
                    == EEvents::eMOUSE_LEFT_BUTTON) {
                if (projectileCooldown <= 0) {
                    MeshHandle meshHandle {};
                    const uint32_t sphereMeshHandleIndex = 2;
                    if (meshHandlers.size() > 2) {
                        meshHandle = meshHandlers[sphereMeshHandleIndex];
                    }

                    TextureHandle textureHandle {};
                    const uint32_t grayTextureHandle = 2;
                    if (textureHandlers.size() > 2) {
                        textureHandle = textureHandlers[grayTextureHandle];
                    }

                    const material material = {
                        .diffuseTextureID_ = textureHandle,
                        .specularTextureID_ = textureHandle,
                        .ambient = {0.05f, 0.05f, 0.05f},
                        .shininess = 128.0f * 0.078125f
                    };

                    const damage damage = {
                        .maximumDamage = 40,
                        .minimumDamage = 20,
                        .criticalHitRate = 0,
                        .criticalModifier = 0
                    };

                    ArchetypeEntityManager* archEntityManager =
                        ArchetypeEntityManager::get_instance();
                    uint64_t projectileEntity =
                        archEntityManager->createEntity();
                    world.addEntityToArchetype(
                        projectileEntity,
                        archView.projectileArchetype
                    );
                    EntityLocation projectileLocation =
                        world.entityLocations[getId(projectileEntity)];

                    CreateProjectile(
                        playerTransform->position,
                        playerView->forward,
                        meshHandle,
                        material,
                        damage,
                        projectileLocation
                    );

                    soundEngine->CreateSoundSample(
                        "../../../examples/assets/sounds/pistol.wav",
                        5,
                        22050,
                        0.05
                    );
                    projectileCooldown = 2.0;
                }
            }
        }
    }

    projectileArchetypesNumber = 0;
    world.searchCacheArchetypes(
        projectileRequiredMask,
        &archView.projectileArchetype,
        projectileArchetypesNumber
    );

    componentsView.projectileTransforms =
        (transform*)archView.projectileArchetype
            ->components[ComponentsIndices::TransformComponent];
    componentsView.projectileColliderFlags =
        (colliderFlags*)archView.projectileArchetype
            ->components[ComponentsIndices::ColliderFlagsComponent];
    componentsView.projectileColliders =
        (collider*)archView.projectileArchetype
            ->components[ComponentsIndices::ColliderComponent];
    componentsView.projectileBundles =
        (ProjectileBundle*)archView.projectileArchetype
            ->components[ComponentsIndices::ProjectileBundleComponent];
    componentsView.projectileHealth =
        (health*)archView.projectileArchetype
            ->components[ComponentsIndices::HealthComponent];
    componentsView.projectileAttacks =
        (attack*)archView.projectileArchetype
            ->components[ComponentsIndices::AttackComponent];

    // Update position of every projectile.
    for (unsigned int x = 0; x < archView.projectileArchetype->entityCount;
         ++x) {
        transform* projectileTransform =
            &componentsView.projectileTransforms[x];
        projectileTransform->position +=
            Normalize(projectileTransform->forward) * cameraSpeed * 2.5;
    }
    // Iterate every projectile, check for collisions with another entities and
    // update damage info if collided entity has attack component.
    for (unsigned int i = 0; i < archView.projectileArchetype->entityCount;
         ++i) {
        colliderFlags* projectileColliderFlags =
            &componentsView.projectileColliderFlags[i];
        health* projectileHealth = &componentsView.projectileHealth[i];
        [[maybe_unused]] attack* projectileAttack =
            &componentsView.projectileAttacks[i];
        const uint8_t wallCollisionBit = 1;
        const uint8_t groundCollistionBit = (1 << 1);
        if ((projectileColliderFlags->flags & wallCollisionBit)
            || (projectileColliderFlags->flags & groundCollistionBit)) {
            damage* projectileDamage =
                &componentsView.projectileBundles[i].damage;
            collider* projectileCollider =
                &componentsView.projectileColliders[i];
            for (unsigned int j = 0; j < projectileCollider->colliders.size();
                 ++j) {
                unsigned int collidedEntity = projectileCollider->colliders[j];

                EntityLocation collidedEntityLocation =
                    world.entityLocations[getId(collidedEntity)];
                uint64_t requiredMask =
                    (1ul << ComponentsIndices::HealthComponent)
                    | (1ul << ComponentsIndices::AttackComponent);

                if ((collidedEntityLocation.arch != nullptr)
                    && (collidedEntityLocation.arch->mask & requiredMask)
                        == requiredMask) {
                    attack* attacks =
                        (attack*)collidedEntityLocation.arch
                            ->components[ComponentsIndices::AttackComponent];
                    attacks[collidedEntityLocation.index].damage =
                        projectileDamage->maximumDamage;
                    projectileHealth->currentHealth = 0;
                }
            }
        }
    }
}
} // namespace glvm

namespace glvm {
void SpatialGridSystem::Update() {
    SpatialGrid& spatialGrid = world.spatialGrid;
    assert(
        spatialGrid.width > 0 && spatialGrid.height > 0 && spatialGrid.depth > 0
    );
    const float chunkSize = spatialGrid.grid[0][0][0].size;

    const float halfWidth = spatialGrid.width * chunkSize * 0.5f;
    const float halfHeight = spatialGrid.height * chunkSize * 0.5f;
    const float halfDepth = spatialGrid.depth * chunkSize * 0.5f;

    cachedArchetypesNumber = 0;
    world.searchCacheArchetypes(
        requiredMask,
        cachedArchetypes,
        cachedArchetypesNumber
    );

    for (uint32_t i0 = 0; i0 < cachedArchetypesNumber; ++i0) {
        Archetype* arch = cachedArchetypes[i0];
        view.transforms =
            (transform*)arch->components[ComponentsIndices::TransformComponent];
        view.meshes = (mesh*)arch->components[ComponentsIndices::MeshComponent];

        for (uint32_t i1 = 0; i1 < arch->entityCount; ++i1) {
            const uint64_t entity = arch->entities[i1];
            EntityLocation& entityLocation =
                world.entityLocations[getId(entity)];
            if (!entityLocation.isDirty && isInitialized) {
                continue;
            }

            if (entityLocation.gridCellCounter > 0) {
                for (uint32_t i2 = 0; i2 < entityLocation.gridCellCounter;
                     ++i2) {
                    uint32_t z = entityLocation.gridCellIndicies[i2][0];
                    uint32_t y = entityLocation.gridCellIndicies[i2][1];
                    uint32_t x = entityLocation.gridCellIndicies[i2][2];
                    std::vector<uint32_t>& chunkEntities =
                        spatialGrid.grid[z][y][x].entities;
                    // Remove by value: the recorded index can be stale after
                    // other removals shifted the cell's vector.
                    for (uint32_t i3 = 0; i3 < chunkEntities.size(); ++i3) {
                        if (chunkEntities[i3] == entity) {
                            chunkEntities.erase(chunkEntities.begin() + i3);
                            break;
                        }
                    }
                }
                entityLocation.gridCellCounter = 0;
            }

            const transform& transform = view.transforms[i1];
            const mesh& mesh = view.meshes[i1];

            MeshHandle entityMeshHandle = mesh.handle;
            MeshAxisMaxAbsoluteValues entityChunkBounds =
                allMeshMaxAbsoluteValues[entityMeshHandle.id];
            std::vector<Vector<float, 3>> entityBoxCornerBoundPoints =
                computeBoxCornerBoundPoints(
                    entityChunkBounds,
                    transform.position,
                    transform.scale
                );

            // Need only left bottom back corner point and right upper front
            // corner point to obtain all box bounds.
            const Vector<float, 3> minEntityPosition =
                entityBoxCornerBoundPoints[0];
            const Vector<float, 3> maxEntityPosition =
                entityBoxCornerBoundPoints[1];

            int indexMinX = static_cast<int>(
                (minEntityPosition[0] + halfWidth) / chunkSize
            );
            int indexMinY = static_cast<int>(
                (minEntityPosition[1] + halfHeight) / chunkSize
            );
            int indexMinZ = static_cast<int>(
                (minEntityPosition[2] + halfDepth) / chunkSize
            );

            int indexMaxX = static_cast<int>(
                (maxEntityPosition[0] + halfWidth) / chunkSize
            );
            int indexMaxY = static_cast<int>(
                (maxEntityPosition[1] + halfHeight) / chunkSize
            );
            int indexMaxZ = static_cast<int>(
                (maxEntityPosition[2] + halfDepth) / chunkSize
            );

            // Entity can legitimately leave the fixed-size world grid (fell off
            // the world edge, projectile flew away) - clamp to nearest edge
            // cell instead of crashing.
            indexMinX = std::clamp(indexMinX, 0, (int)spatialGrid.width - 1);
            indexMinY = std::clamp(indexMinY, 0, (int)spatialGrid.height - 1);
            indexMinZ = std::clamp(indexMinZ, 0, (int)spatialGrid.depth - 1);
            indexMaxX = std::clamp(indexMaxX, 0, (int)spatialGrid.width - 1);
            indexMaxY = std::clamp(indexMaxY, 0, (int)spatialGrid.height - 1);
            indexMaxZ = std::clamp(indexMaxZ, 0, (int)spatialGrid.depth - 1);

            for (auto i2 = indexMinZ; i2 <= indexMaxZ; ++i2) {
                for (auto i3 = indexMinY; i3 <= indexMaxY; ++i3) {
                    for (auto i4 = indexMinX; i4 <= indexMaxX; ++i4) {
                        std::vector<uint32_t>& chunkEntities =
                            spatialGrid.grid[i2][i3][i4].entities;
                        if (!isExist<uint32_t>(chunkEntities, entity)) {
                            chunkEntities.push_back(entity);
                            const uint32_t currentGridCell =
                                entityLocation.gridCellCounter;
                            assert(
                                currentGridCell < 8
                            ); ///< 8 is a maximum number for 1 entity to exist
                               ///< in grid cell
                            entityLocation.gridCellIndicies[currentGridCell] =
                                Vector<float, 3>(i2, i3, i4);
                            entityLocation.cellEntityIndices[currentGridCell] =
                                chunkEntities.size() - 1;
                            entityLocation.isDirty = false;
                            ++entityLocation.gridCellCounter;
                        }
                    }
                }
            }
        }
    }
    if (!isInitialized) {
        isInitialized = true;
    }
}

}; // namespace glvm

namespace glvm {
TextureManager* TextureManager::pInstance_ = nullptr;
std::mutex TextureManager::Mutex_;

TextureManager::TextureManager() = default;

void TextureManager::BindTexture(
    unsigned int _entityID,
    unsigned int _textureID
) {
    textureVector_[_textureID].entitiesOwnsThisTypeOfTexture_.push_back(
        _entityID
    );
}

TextureManager* TextureManager::get_instance() {
    std::lock_guard<std::mutex> lock(Mutex_);
    if (pInstance_ == nullptr) {
        pInstance_ = new TextureManager();
    }
    return pInstance_;
}

void TextureManager::SetTextureVector(std::vector<Texture> _textureVector) {
    textureVector_ = _textureVector;
}

std::vector<Texture>& TextureManager::GetTextureVector() {
    return textureVector_;
}
} // namespace glvm

ThreadPool::ThreadPool(size_t numThreads) : stop(false) {
    for (size_t i = 0; i < numThreads; ++i) {
        workers.emplace_back([this] {
            while (true) {
                std::function<void()> task;
                {
                    std::unique_lock<std::mutex> lock(this->queueMutex);
                    this->condition.wait(lock, [this] {
                        return this->stop || !this->tasks.empty();
                    });

                    if (this->stop && this->tasks.empty()) {
                        return;
                    }

                    task = std::move(this->tasks.front());
                    this->tasks.pop();
                }
                task();
            }
        });
    }
}

ThreadPool::~ThreadPool() {
    {
        std::unique_lock<std::mutex> lock(queueMutex);
        stop = true;
    }
    condition.notify_all();
    for (std::thread& worker : workers) {
        worker.join();
    }
}

#ifdef __linux__
#endif
#ifdef _WIN32
#endif

namespace glvm {
IChrono* CTimerCreator::Create() {
#ifdef __linux__
    return new CTimerX;
#endif

#ifdef _WIN32
    return new CTimerWin;
#endif
}
} // namespace glvm

#ifdef _WIN32

namespace glvm {
CTimerWin::CTimerWin() {
    InitFrequency();
    Reset();
}

double CTimerWin::InitFrequency() {
    QueryPerformanceFrequency((PLARGE_INTEGER)&i64Freq_);
    return (double)i64Freq_;
}

double CTimerWin::Reset() {
    QueryPerformanceCounter((PLARGE_INTEGER)&i64Start_);
    return (double)i64Start_;
}

double CTimerWin::GetElapsed() {
    QueryPerformanceCounter((PLARGE_INTEGER)&i64Now_);
    return (double)(i64Now_ - i64Start_) / i64Freq_;
}
} // namespace glvm
#endif // _WIN32

#ifdef _WIN32

namespace glvm {
void CSoundEngineWaveform::SoundStream() {
    for (unsigned int i = 0; i < tSound_Container.size(); ++i) {
        PlaybackSoundSample(*tSound_Container[i]);
        tSound_Container.erase(tSound_Container.begin() + i);
    }
}

void CSoundEngineWaveform::PlaybackSoundSample(CSoundSample& _sound_sample) {
    HWAVEOUT hWaveOut;
    WAVEHDR lpWaveHdr {};
    WAVEFORMATEX Format;
    Format.wFormatTag = WAVE_FORMAT_PCM;
    Format.nChannels = 2;
    Format.nSamplesPerSec = _sound_sample.uiRate_;
    Format.nAvgBytesPerSec = Format.nSamplesPerSec * Format.nChannels * 2;
    // Change this field first if got any problems.
    Format.nBlockAlign = 4;
    Format.wBitsPerSample = 16;
    Format.cbSize = 0;
    // Open a waveform device for output using window callback.
    unsigned int rc = 0;
    rc = waveOutOpen(&hWaveOut, WAVE_MAPPER, &Format, 0L, 0L, 0L);
    if (rc != MMSYSERR_NOERROR) {
        std::cerr << "waveOutOpen: " << "error code: " << rc << std::endl;
        std::exit(-1);
    }

    std::ifstream file(
        _sound_sample.kPath_to_File_,
        std::ios_base::binary | std::ios_base::in
    );
    if (!file) {
        std::cerr << "Fail to open file." << std::endl;
        std::exit(-1);
    }

    char* buf = (char*)malloc(Format.nAvgBytesPerSec * 2);
    while (true) {
        file.read(buf, Format.nAvgBytesPerSec * 2);
        if (file.gcount() == 0) {
            break;
        }

        lpWaveHdr.lpData = buf;
        lpWaveHdr.dwBufferLength = file.gcount();
        lpWaveHdr.dwFlags = 0L;
        lpWaveHdr.dwLoops = 0L;
        waveOutPrepareHeader(hWaveOut, &lpWaveHdr, sizeof(WAVEHDR));
        waveOutWrite(hWaveOut, &lpWaveHdr, sizeof(WAVEHDR));
        Sleep((lpWaveHdr.dwBufferLength * 1000) / (Format.nAvgBytesPerSec * 2));
        waveOutUnprepareHeader(hWaveOut, &lpWaveHdr, sizeof(WAVEHDR));
    }

    free(buf);
    waveOutClose(hWaveOut);
}

void CSoundEngineWaveform::SetMasterVolume(long /* l_volume */) {}

std::vector<CSoundSample*>& CSoundEngineWaveform::GetSoundContainer() {
    return tSound_Container;
}
} // namespace glvm
#endif // _WIN32

#ifdef _WIN32

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(
    HWND hWnd,
    UINT msg,
    WPARAM wParam,
    LPARAM lParam
);

namespace glvm {
WindowWinVulkan* WindowWinVulkan::instance = nullptr;

WindowWinVulkan::WindowWinVulkan() {
    instance = this;
    const char* _title = "Game";
    int _width = width / 2, _height = height / 2;
    // Register the window class for the main window.
    window_Class_.style = 0;
    window_Class_.lpfnWndProc = MainWndProc;
    window_Class_.cbClsExtra = 0;
    window_Class_.cbWndExtra = 0;
    window_Class_.hInstance = NULL;
    window_Class_.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    window_Class_.hCursor = LoadCursor(NULL, NULL);
    window_Class_.hbrBackground = NULL;
    window_Class_.lpszMenuName = NULL;
    window_Class_.lpszClassName = "Game";

    RegisterClassA(&window_Class_);

    DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX
        | WS_MAXIMIZEBOX | WS_THICKFRAME;

    RECT rect;
    SetRect(&rect, 0, 0, _width, _height);
    AdjustWindowRect(&rect, style, FALSE);
    int _x_position = (width - (rect.right - rect.left)) / 2;
    int _y_position = (height - (rect.bottom - rect.top)) / 2;

    // Create the main window.
    pModern_Window_ = CreateWindowA(
        "Game",
        _title,
        style,
        _x_position,
        _y_position,
        rect.right - rect.left,
        rect.bottom - rect.top,
        (HWND)NULL,
        (HMENU)NULL,
        NULL,
        (LPVOID)NULL
    );

    // Show the window and paint its contents.
    ShowWindow(pModern_Window_, SW_SHOWDEFAULT);
    UpdateWindow(pModern_Window_);
}

void WindowWinVulkan::SwapBuffers() {}

void WindowWinVulkan::ClearDisplay() {}

bool WindowWinVulkan::HandleEvent(CEvent& _Event) {
    // Create message struct object.
    MSG msg;

    SetWindowLongPtrW(pModern_Window_, GWLP_USERDATA, (LONG_PTR)&_Event);
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
        Input_Stack_->ControlInput(_Event);
        // DispatchMessage may not have set the event (e.g. a WM_CHAR left
        // over from TranslateMessage, or WM_KEYUP for an unhandled key). The
        // stale value would otherwise be re-pushed by ControlInput on the next
        // message/frame and toggle the cursor a second time.
        _Event.SetEvent(EEvents::eDEFAULT);
    }
    return false;
}

void WindowWinVulkan::Close() {
    DestroyWindow(pModern_Window_);
    PostQuitMessage(0);
}

HWND WindowWinVulkan::GetClassicWindowHWND() {
    return pClassic_Window_;
}

HWND WindowWinVulkan::GetModernWindowHWND() {
    return pModern_Window_;
}

void WindowWinVulkan::CursorLock(
    int _x_position,
    int _y_position,
    int* _x_offset,
    int* _y_offset
) {
    RECT clientRect;
    GetClientRect(pModern_Window_, &clientRect);
    const int centerX = clientRect.right / 2;
    const int centerY = clientRect.bottom / 2;
    POINT point_position {centerX, centerY};
    ClientToScreen(pModern_Window_, &point_position);
    // Solve a problem with endlessly growing numbers in the start game run.
    if (_x_position > clientRect.right || _x_position < 0
        || _y_position > clientRect.bottom || _y_position < 0) {
        return;
    }
    int iOffset_X = 0, iOffset_Y = 0;
    iOffset_X = _x_position - previous_X;
    iOffset_Y = _y_position - previous_Y;
    previous_X = _x_position;
    previous_Y = _y_position;

    // The per-frame delta is measured against the cursor's actual position
    // after the previous warp, not against the computed center: the two can
    // differ by a few pixels (DPI rounding), and accumulating that constant
    // error would slowly drift the view until it hits the pitch clamp below.
    // A >250px jump between frames is a cursor teleport, not mouse movement:
    // discard it so the camera doesn't snap toward the new position (startup,
    // refocus, stale sample after the warp, and the first sample after the
    // inventory closes - the cursor was free while the inventory was open).
    // Discard the sample, just re-warp to the center.
    if (iOffset_X > 250 || iOffset_X < -250 || iOffset_Y > 250
        || iOffset_Y < -250) {
    } else {
        *_x_offset += iOffset_X;
        *_y_offset -= iOffset_Y;
    }
    // Pitch is limited by angle in Engine::SetViewMatrix(), so this offset may
    // accumulate freely; no pixel clamp here (resolution-independent).
    SetCursorPos(point_position.x, point_position.y);
    SetCursor(NULL);
    // Baseline for the next frame: where the cursor actually ended up after the
    // warp (matches what the next WM_MOUSEMOVE will report).
    POINT actual_position;
    GetCursorPos(&actual_position);
    ScreenToClient(pModern_Window_, &actual_position);
    previous_X = actual_position.x;
    previous_Y = actual_position.y;
}

// Callback method for events handling.
LRESULT CALLBACK WindowWinVulkan::MainWndProc(
    HWND _pHwnd,
    UINT _pMsg,
    WPARAM _pWParam,
    LPARAM _pLParam
) {
    ImGui_ImplWin32_WndProcHandler(_pHwnd, _pMsg, _pWParam, _pLParam);
    CEvent* pEvent = (CEvent*)GetWindowLongPtrW(_pHwnd, GWLP_USERDATA);

    if (_pMsg == WM_KEYDOWN && _pWParam == VK_ESCAPE && pEvent != nullptr
        && (_pLParam & (1 << 30)) == 0) {
        pEvent->SetEvent(EEvents::eCURSOR_RELEASED);
        return 0;
    }

    if (ImGui::GetCurrentContext()) {
        ImGuiIO& io = ImGui::GetIO();
        const bool isMouseMessage =
            (_pMsg >= WM_MOUSEFIRST && _pMsg <= WM_MOUSELAST)
            || _pMsg == WM_MOUSEWHEEL || _pMsg == WM_MOUSEHWHEEL;
        const bool isKeyboardMessage =
            (_pMsg >= WM_KEYFIRST && _pMsg <= WM_KEYLAST) || _pMsg == WM_CHAR
            || _pMsg == WM_SYSCHAR || _pMsg == WM_SYSKEYDOWN
            || _pMsg == WM_SYSKEYUP;
        if ((isMouseMessage && io.WantCaptureMouse)
            || (isKeyboardMessage && io.WantCaptureKeyboard)) {
            return 0;
        }
    }

    if (pEvent == nullptr) {
        return DefWindowProcA(_pHwnd, _pMsg, _pWParam, _pLParam);
    }
    int iMouse_Position_X, iMouse_Position_Y;
    switch (_pMsg) {
        case WM_CREATE:
            return 0;
        case WM_SIZE:
            return 0;
        case WM_LBUTTONDOWN:
            pEvent->SetEvent(EEvents::eMOUSE_LEFT_BUTTON);
            return 0;

        case WM_SETFOCUS:
            if (WindowWinVulkan::instance) {
                WindowWinVulkan::instance->isFocused = true;
            }
            return 0;
        case WM_KILLFOCUS:
            if (WindowWinVulkan::instance) {
                WindowWinVulkan::instance->isFocused = false;
            }
            return 0;
        case WM_LBUTTONUP:
            pEvent->SetEvent(EEvents::eMOUSE_LEFT_BUTTON_RELEASE);
            pEvent->isLeftMouseButtonReleased = true;
            return 0;
        case WM_MOUSEMOVE:
            iMouse_Position_X = GET_X_LPARAM(_pLParam);
            iMouse_Position_Y = GET_Y_LPARAM(_pLParam);
            pEvent->SetEvent(EEvents::eMOUSE_POINTER_POSITION);
            pEvent->mousePointerPosition.position_X = iMouse_Position_X;
            pEvent->mousePointerPosition.position_Y = iMouse_Position_Y;
            return 0;
        case WM_KEYDOWN:
            switch (_pWParam) {
                case VK_LEFT:
                    break;
                case VK_RIGHT:
                    break;
                case VK_ESCAPE:
                    break;
                case VK_W:
                    pEvent->SetEvent(EEvents::eMOVE_FORWARD);
                    break;
                case VK_S:
                    pEvent->SetEvent(EEvents::eMOVE_BACKWARD);
                    break;
                case VK_A:
                    pEvent->SetEvent(EEvents::eMOVE_LEFT);
                    break;
                case VK_D:
                    pEvent->SetEvent(EEvents::eMOVE_RIGHT);
                    break;
                case VK_SPACE:
                    pEvent->SetEvent(EEvents::eJUMP);
                    break;
                case VK_I:
                    pEvent->SetEvent(EEvents::eINVENTORY);
                    break;
                case VK_UP:
                    break;
                case VK_DOWN:
                    break;
                case VK_HOME:
                    break;
                case VK_END:
                    break;
                case VK_INSERT:
                    break;
                case VK_DELETE:
                    break;
                case VK_F2:
                    break;
                default:
                    break;
            }
            break;
        case WM_KEYUP:
            switch (_pWParam) {
                case VK_LEFT:
                    break;
                case VK_RIGHT:
                    break;
                case VK_W:
                    pEvent->SetEvent(EEvents::eKEYRELEASE_W);
                    break;
                case VK_S:
                    pEvent->SetEvent(EEvents::eKEYRELEASE_S);
                    break;
                case VK_A:
                    pEvent->SetEvent(EEvents::eKEYRELEASE_A);
                    break;
                case VK_D:
                    pEvent->SetEvent(EEvents::eKEYRELEASE_D);
                    break;
                case VK_SPACE:
                    pEvent->SetEvent(EEvents::eKEYRELEASE_JUMP);
                    break;
                case VK_I:
                    pEvent->SetEvent(EEvents::eINVENTORY_RELEASE);
                    break;
                case VK_UP:
                    break;
                case VK_DOWN:
                    break;
                case VK_HOME:
                    break;
                case VK_END:
                    break;
                case VK_INSERT:
                    break;
                case VK_DELETE:
                    break;
                case VK_F2:
                    break;
                default:
                    break;
            }
            break;
        case WM_CLOSE:
            pEvent->SetEvent(EEvents::eGAME_LOOP_KILL);
            return 0;
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        // Process other messages.
        default:
            return DefWindowProc(_pHwnd, _pMsg, _pWParam, _pLParam);
    }
    return 0;
}
} // namespace glvm
#endif // _WIN32

const int kVertex_Size = 9;
constexpr float kWidth_Offset = 1.0f / 3;

float aVertices[kVertex_Size] = {
    -0.5f,
    -0.5f,
    0.5f, // Left vertex.
    -0.5f,
    0.5f,
    0.0f, // Right vertex.
    0.0f,
    0.0f,
    0.0f // Upper vertex.
};

float aVertices2[kVertex_Size] = {
    0.5f,
    -0.5f,
    // Left vertex.
    -0.5f,
    0.5f,
    0.5f,
    // Right vertex.
    0.5f,
    0.0f,
    0.0f,
    // Upper vertex.
    -1.0f
};

float aVertices_Static_Object[] = {
    // Coordinates.
    0.5f,  0.5f,  0.0f, 1.0f, 1.0f, // Up right vertex.
    0.5f,  -0.5f, 0.0f, 1.0f, 0.0f, // Bottom right vertex.
    -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, // Bottom left vertex.
    -0.5f, 0.5f,  0.0f, 0.0f, 1.0f, // Up left vertex.
    0.5f,  0.5f,  0.0f, 1.0f, 1.0f, -0.5f, -0.5f, 0.0f, 0.0f, 0.0f
};

float vertices[] = {
    // Coordinates.
    0.5f,  0.5f,  0.0f, kWidth_Offset, 1.0f, // Up right vertex.
    0.5f,  -0.5f, 0.0f, kWidth_Offset, 0.75f, // Bottom right vertex.
    -0.5f, -0.5f, 0.0f, 0.0f,          0.75f, // Bottom left vertex.
    -0.5f, 0.5f,  0.0f, 0.0f,          1.0f, // Up left vertex.
    0.5f,  0.5f,  0.0f, kWidth_Offset, 1.0f,  -0.5f, -0.5f, 0.0f, 0.0f, 0.75f
};

float vertices2[] = {
    // Coordinates.
    0.5f,  0.5f,  0.0f, kWidth_Offset * 2, 1.0f, // Up right vertex.
    0.5f,  -0.5f, 0.0f, kWidth_Offset * 2, 0.75f, // Bottom right vertex.
    -0.5f, -0.5f, 0.0f, kWidth_Offset,     0.75f, // Bottom left vertex.
    -0.5f, 0.5f,  0.0f, kWidth_Offset,     1.0f, // Up left vertex.
    0.5f,  0.5f,  0.0f, kWidth_Offset * 2, 1.0f,
    -0.5f, -0.5f, 0.0f, kWidth_Offset,     0.75f
};

float vertices3[] = {
    // Coordinates.
    0.5f,
    0.5f,
    0.0f,
    1.0f,
    1.0f, // Up right vertex.
    0.5f,
    -0.5f,
    0.0f,
    1.0f,
    0.75f, // Bottom right vertex.
    -0.5f,
    -0.5f,
    0.0f,
    kWidth_Offset * 2,
    0.75f, // Bottom left vertex.
    -0.5f,
    0.5f,
    0.0f,
    kWidth_Offset * 2,
    1.0f, // Up left vertex.
    0.5f,
    0.5f,
    0.0f,
    1.0f,
    1.0f,
    -0.5f,
    -0.5f,
    0.0f,
    kWidth_Offset * 2,
    0.75f
};

float vertices4[] = {
    // Coordinates.
    0.5f,  0.5f,  0.0f, kWidth_Offset, 0.75f, // Up right vertex.
    0.5f,  -0.5f, 0.0f, kWidth_Offset, 0.5f, // Bottom right vertex.
    -0.5f, -0.5f, 0.0f, 0.0f,          0.5f, // Bottom left vertex.
    -0.5f, 0.5f,  0.0f, 0.0f,          0.75f, // Up left vertex.
    0.5f,  0.5f,  0.0f, kWidth_Offset, 0.75f, -0.5f, -0.5f, 0.0f, 0.0f, 0.5f
};

float vertices5[] = {
    // Coordinates.
    0.5f,  0.5f,  0.0f, kWidth_Offset * 2, 0.75f, // Up right vertex.
    0.5f,  -0.5f, 0.0f, kWidth_Offset * 2, 0.5f, // Bottom right vertex.
    -0.5f, -0.5f, 0.0f, kWidth_Offset,     0.5f, // Bottom left vertex.
    -0.5f, 0.5f,  0.0f, kWidth_Offset,     0.75f, // Up left vertex.
    0.5f,  0.5f,  0.0f, kWidth_Offset * 2, 0.75f,
    -0.5f, -0.5f, 0.0f, kWidth_Offset,     0.5f
};

float vertices6[] = {
    // Coordinates.
    0.5f,
    0.5f,
    0.0f,
    1.0f,
    0.75f, // Up right vertex.
    0.5f,
    -0.5f,
    0.0f,
    1.0f,
    0.5f, // Bottom right vertex.
    -0.5f,
    -0.5f,
    0.0f,
    kWidth_Offset * 2,
    0.5f, // Bottom left vertex.
    -0.5f,
    0.5f,
    0.0f,
    kWidth_Offset * 2,
    0.75f, // Up left vertex.
    0.5f,
    0.5f,
    0.0f,
    1.0f,
    0.75f,
    -0.5f,
    -0.5f,
    0.0f,
    kWidth_Offset * 2,
    0.5f
};

float vertices7[] = {
    // Coordinates.
    0.5f,  0.5f,  0.0f, kWidth_Offset, 0.5f, // Up right vertex.
    0.5f,  -0.5f, 0.0f, kWidth_Offset, 0.25f, // Bottom right vertex.
    -0.5f, -0.5f, 0.0f, 0.0f,          0.25f, // Bottom left vertex.
    -0.5f, 0.5f,  0.0f, 0.0f,          0.5f, // Up left vertex.
    0.5f,  0.5f,  0.0f, kWidth_Offset, 0.5f,  -0.5f, -0.5f, 0.0f, 0.0f, 0.25f
};

float vertices8[] = {
    // Coordinates.
    0.5f,  0.5f,  0.0f, kWidth_Offset * 2, 0.5f, // Up right vertex.
    0.5f,  -0.5f, 0.0f, kWidth_Offset * 2, 0.25f, // Bottom right vertex.
    -0.5f, -0.5f, 0.0f, kWidth_Offset,     0.25f, // Bottom left vertex.
    -0.5f, 0.5f,  0.0f, kWidth_Offset,     0.5f, // Up left vertex.
    0.5f,  0.5f,  0.0f, kWidth_Offset * 2, 0.5f,
    -0.5f, -0.5f, 0.0f, kWidth_Offset,     0.25f
};

float vertices9[] = {
    // Coordinates.
    0.5f,
    0.5f,
    0.0f,
    1.0f,
    0.5f, // Up right vertex.
    0.5f,
    -0.5f,
    0.0f,
    1.0f,
    0.25f, // Bottom right vertex.
    -0.5f,
    -0.5f,
    0.0f,
    kWidth_Offset * 2,
    0.25f, // Bottom left vertex.
    -0.5f,
    0.5f,
    0.0f,
    kWidth_Offset * 2,
    0.5f, // Up left vertex.
    0.5f,
    0.5f,
    0.0f,
    1.0f,
    0.5f,
    -0.5f,
    -0.5f,
    0.0f,
    kWidth_Offset * 2,
    0.25f
};

float vertices10[] = {
    // Coordinates.
    0.5f,  0.5f,  0.0f, kWidth_Offset, 0.25f, // Up right vertex.
    0.5f,  -0.5f, 0.0f, kWidth_Offset, 0.0f, // Bottom right vertex.
    -0.5f, -0.5f, 0.0f, 0.0f,          0.0f, // Bottom left vertex.
    -0.5f, 0.5f,  0.0f, 0.0f,          0.25f, // Up left vertex.
    0.5f,  0.5f,  0.0f, kWidth_Offset, 0.25f, -0.5f, -0.5f, 0.0f, 0.0f, 0.0f
};

float vertices11[] = {
    // Coordinates.
    0.5f,  0.5f,  0.0f, kWidth_Offset * 2, 0.25f, // Up right vertex.
    0.5f,  -0.5f, 0.0f, kWidth_Offset * 2, 0.0f, // Bottom right vertex.
    -0.5f, -0.5f, 0.0f, kWidth_Offset,     0.0f, // Bottom left vertex.
    -0.5f, 0.5f,  0.0f, kWidth_Offset,     0.25f, // Up left vertex.
    0.5f,  0.5f,  0.0f, kWidth_Offset * 2, 0.25f,
    -0.5f, -0.5f, 0.0f, kWidth_Offset,     0.0f
};

float vertices12[] = {
    // Coordinates.
    0.5f,
    0.5f,
    0.0f,
    1.0f,
    0.25f, // Up right vertex.
    0.5f,
    -0.5f,
    0.0f,
    1.0f,
    0.0f, // Bottom right vertex.
    -0.5f,
    -0.5f,
    0.0f,
    kWidth_Offset * 2,
    0.0f, // Bottom left vertex.
    -0.5f,
    0.5f,
    0.0f,
    kWidth_Offset * 2,
    0.25f, // Up left vertex.
    0.5f,
    0.5f,
    0.0f,
    1.0f,
    0.25f,
    -0.5f,
    -0.5f,
    0.0f,
    kWidth_Offset * 2,
    0.0f
};

int Vertices_Size = sizeof(vertices);

namespace glvm {
static bool equalsCStr(const std::vector<char>& v, const char* s) {
    return strcmp(v.data(), s) == 0;
}

CWaveFrontObjParser::CWaveFrontObjParser() {}

const std::vector<SVertex>& CWaveFrontObjParser::getCoordinateVertices() const {
    return coordinateVertices_;
}

const std::vector<SVertex>& CWaveFrontObjParser::getTextureVertices() const {
    return textureVertices_;
}

const std::vector<SVertex>& CWaveFrontObjParser::getNormals() const {
    return normals_;
}

const std::vector<SFace>& CWaveFrontObjParser::getFaces() const {
    return faces_;
}

void CWaveFrontObjParser::ReadFile(const char* _filePath) {
    const char* _pWavefrontObjFile = _filePath;
    std::ifstream WavefrontObjFileInputStream;
    std::stringstream WavefrontObjFileOutputStream;

    WavefrontObjFileInputStream.open(_pWavefrontObjFile);
    if (WavefrontObjFileInputStream.good()) {
        WavefrontObjFileOutputStream << WavefrontObjFileInputStream.rdbuf();
        WavefrontObjFileInputStream.close();
        sWavefrontObjFileData = WavefrontObjFileOutputStream.str();
    } else {
        std::cout << "Error of reading " << _filePath << " file" << std::endl;
        return;
    }

    pWavefrontObjFileData = sWavefrontObjFileData.c_str();
}

void CWaveFrontObjParser::ParseFile() {
    while (pWavefrontObjFileData[uiCounter] != '\0') {
        std::vector<std::vector<char>> line =
            Split(pWavefrontObjFileData, ' ', '\n', uiCounter);
        if (equalsCStr(line[0], "v")) {
            SVertex vertex = ParseVertices(line);
            coordinateVertices_.push_back(vertex);
        }
        if (equalsCStr(line[0], "vt")) {
            SVertex vertex = ParseVertices(line);
            textureVertices_.push_back(vertex);
        }
        if (equalsCStr(line[0], "vn")) {
            SVertex vertex = ParseVertices(line);
            normals_.push_back(vertex);
        }
        if (equalsCStr(line[0], "f")) {
            SFace face = ParseFaces(line);
            faces_.push_back(face);
        }
    }
}

std::vector<std::vector<char>> CWaveFrontObjParser::Split(
    const char* _pWaveFrontObjFileData,
    const char _separator,
    const char _exitSymbol,
    unsigned int& _uiCounter
) {
    std::vector<std::vector<char>> wordsContainer;
    unsigned int outerIndex = 0;
    wordsContainer.push_back({});

    for (;; ++_uiCounter) {
        if (_pWaveFrontObjFileData[_uiCounter] == '#') {
            while (_pWaveFrontObjFileData[_uiCounter] != '\n') {
                ++_uiCounter;
            }
            continue;
        }
        if (_pWaveFrontObjFileData[_uiCounter] == _separator) {
            wordsContainer[outerIndex].push_back('\0');
            wordsContainer.push_back({});
            ++outerIndex;
            continue;
        }
        if (_pWaveFrontObjFileData[_uiCounter] == _exitSymbol) {
            ++_uiCounter;
            wordsContainer[outerIndex].push_back('\0');
            return wordsContainer;
        }
        wordsContainer[outerIndex].push_back(_pWaveFrontObjFileData[_uiCounter]);
    }
}

SVertex CWaveFrontObjParser::ParseVertices(
    std::vector<std::vector<char>> _wordsContainer
) {
    SVertex vertex;
    unsigned int uiVertexIndex = 0;

    unsigned int uiWordsContainerSize = _wordsContainer.size();
    for (unsigned int i = 1; i < uiWordsContainerSize; ++i) {
        float floatNumber = ParseFloating(_wordsContainer[i]);
        vertex[uiVertexIndex++] = floatNumber;
    }

    return vertex;
}

SFace CWaveFrontObjParser::ParseFaces(
    std::vector<std::vector<char>> _wordsContainer
) {
    SFace face;
    std::vector<std::vector<char>> wordsInnerContainer;
    std::vector<char> word;

    unsigned int uiWordsContainerSize = _wordsContainer.size();

    for (unsigned int i = 1; i < uiWordsContainerSize; ++i) {
        unsigned int counter = 0;
        wordsInnerContainer =
            Split(_wordsContainer[i].data(), '/', '\0', counter);

        for (unsigned int j = 0; j < wordsInnerContainer.size(); ++j) {
            word = wordsInnerContainer[j];
            int iValue = ParseInteger(word);

            face[j].push_back(iValue);
        }
    }
    return face;
}

int CWaveFrontObjParser::ParseInteger(std::vector<char> _word) {
    std::vector<int> baseContainer;

    for (unsigned int i = 0; i < _word.size() - 1; ++i) {
        baseContainer.push_back(_word[i] - 48);
    }

    int iResult = 0;
    bool negateFlag = false;

    unsigned int baseContainerSize = baseContainer.size();
    for (unsigned int i = 0; i < baseContainerSize; ++i) {
        if (negateFlag && i == 0) {
            continue;
        } else if (baseContainer[i] == -5 && i == 0) {
            continue;
        }

        iResult += baseContainer[i] * std::pow(10, (baseContainerSize - 1) - i);
    }

    return iResult;
}

float CWaveFrontObjParser::ParseFloating(std::vector<char> _word) {
    std::vector<int> baseContainer;

    for (unsigned int i = 0; i < _word.size() - 1; ++i) {
        baseContainer.push_back(_word[i] - 48);
    }

    int integerPart = 0;
    float floatingPart = 0;
    std::vector<int> integerPartContainer;
    std::vector<int> floatingPartContainer;
    bool dotFlag = false;
    bool negateFlag = false;
    unsigned int baseContainerSize = baseContainer.size();

    if (baseContainer[0] == -3) {
        negateFlag = true;
    }

    for (unsigned int i = 0; i < baseContainerSize; ++i) {
        if (negateFlag && i == 0) {
            continue;
        } else if (baseContainer[i] == -5 && i == 0) {
            continue;
        } else if (baseContainer[i] == -2) {
            dotFlag = true;
            continue;
        }

        if (baseContainer[i] >= 0 && baseContainer[i] <= 9) {
            if (dotFlag) {
                floatingPartContainer.push_back(baseContainer[i]);
            } else {
                integerPartContainer.push_back(baseContainer[i]);
            }
        } else {
            std::cout << "Element is not a number" << std::endl;
            return NAN;
        }
    }

    unsigned int integerPartContainerSize = integerPartContainer.size();
    for (unsigned int i = 0; i < integerPartContainerSize; ++i) {
        integerPart += integerPartContainer[i]
            * std::pow(10, (integerPartContainerSize - 1) - i);
    }

    unsigned int floatingPartContainerSize = floatingPartContainer.size();
    for (unsigned int i = 0; i < floatingPartContainerSize; ++i) {
        floatingPart += floatingPartContainer[i] / std::pow(10, i + 1);
    }

    float result = 0;
    result = (float)(integerPart + floatingPart);

    if (negateFlag) {
        result *= -1.0f;
    }

    return result;
}
} // namespace glvm
