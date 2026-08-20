#pragma once

#include "glvm/archetype_ecs/arch_ecs_utils.hpp"
#include "glvm/archetype_ecs/arch_ecs_world.hpp"
#include "glvm/archetype_ecs/archetype_interface.hpp"
#include "glvm/archetypes/directional_light_archetype.hpp"
#include "glvm/archetypes/enemy_archetype.hpp"
#include "glvm/archetypes/item_archetype.hpp"
#include "glvm/archetypes/level_chunk_archetype.hpp"
#include "glvm/archetypes/player_archetype.hpp"
#include "glvm/archetypes/point_light_archetype.hpp"
#include "glvm/archetypes/projectile_archetype.hpp"
#include "glvm/archetypes/rigid_body_archetype.hpp"
#include "glvm/archetypes/spot_light_archetype.hpp"
#include "glvm/archetypes/static_mesh_archetype.hpp"
#include "glvm/component_manager.hpp"
#include "glvm/components/inventory_component.hpp"
#include "glvm/components_full_set.hpp"
#include "glvm/constants.hpp"
#include "glvm/entity_manager.hpp"
#include "glvm/event.hpp"
#include "glvm/events_stack.hpp"
#include "glvm/graphic_api/vulkan.hpp"
#include "glvm/i_chrono.hpp"
#include "glvm/i_sound_engine.hpp"
#include "glvm/i_window.hpp"
#include "glvm/procedural_level_generating_system.hpp"
#include "glvm/shader_structs.hpp"
#include "glvm/system_manager.hpp"
#include "glvm/systems/damage_system.hpp"
#include "glvm/systems/enemy_system.hpp"
#include "glvm/systems/inventory_system.hpp"
#include "glvm/systems/item_system.hpp"
#include "glvm/systems/spatial_grid_system.hpp"
#include "glvm/systems_full_set.hpp"
#include "glvm/texture.hpp"
#include "glvm/texture_manager.hpp"
#include "glvm/timer_creator.hpp"
#include "glvm/vk_structs.hpp"

#include <cstdint>
#include <mutex>
#include <vector>

namespace glvm::core {
enum RendererType { VULKAN_RENDERER };

class Engine {
    static Engine* pInstance_;
    static std::mutex Mutex_;

    Time::IChrono* chrono;
    Sound::ISoundEngine* soundEngine;
    std::thread sound_thread;
    std::atomic<bool> runningSound {false};
    float deltaFrameTime;
    float gravity;
    bool isLeftMouseButtonPressed;
    std::vector<ecs::Texture> textureVector;
    std::vector<const char*> pathsArray_;
    std::vector<const char*> pathsGLTF_;
    uint32_t meshID = 0;
    bool isAlreadyCached;
    bool isInventoryKeyHeld = false;
    bool wasInventoryOpened = false;
    bool isCursorHidden = false;
    float hud_screen_x = 0.0f;
    float hud_screen_y;
    // If don't have any dragged item then this variable have value of -1.
    int dragedItemEntity = -1;
    float fYaw = -90.0f;
    float fPitch = 0.0f;
    float previousMouseOffsetX = 0.0f;
    float previousMouseOffsetY = 0.0f;
    CVulkanRenderer* vulkanRenderer;
    ecs::SpatialGridSystem* spatialGridSystem;
    ecs::CCollisionSystem* collisionSystem;
    ecs::CMovementSystem* movementSystem;
    ecs::CPhysicsSystem* physicsSystem;
    ecs::CProjectileSystem* projectileSystem;
    ecs::DamageSystem* damageSystem;
    ecs::EnemySystem* enemySytem;
    ecs::ItemSystem* itemSystem;
    ProceduralLevelGeneratingSystem* procuduralLevelGeneratingSystem;
    ecs::InventorySystem* inventorySystem;
    ecs::arch::Archetype* cachedDirectionalLigthArchetypes[32];
    uint32_t directionalLightArchetypesNumber = 0;
    uint64_t directionalLightRequiredMask =
        (1ul << ecs::arch::ComponentsIndices::DIRECTIONAL_LIGHT_COMPONENT)
        | (1ul << ecs::arch::ComponentsIndices::MESH_COMPONENT)
        | (1ul << ecs::arch::ComponentsIndices::TRANSFORM_COMPONENT);
    ecs::arch::Archetype* cachedSpotLigthArchetypes[32];
    uint32_t spotLightArchetypesNumber = 0;
    uint64_t spotLightRequiredMask =
        (1ul << ecs::arch::ComponentsIndices::SPOT_LIGHT_COMPONENT)
        | (1ul << ecs::arch::ComponentsIndices::MESH_COMPONENT)
        | (1ul << ecs::arch::ComponentsIndices::TRANSFORM_COMPONENT);
    ecs::arch::Archetype* cachedPointLigthArchetypes[32];
    uint32_t pointLightArchetypesNumber = 0;
    uint64_t pointLightRequiredMask =
        (1ul << ecs::arch::ComponentsIndices::POINT_LIGHT_COMPONENT)
        | (1ul << ecs::arch::ComponentsIndices::MESH_COMPONENT)
        | (1ul << ecs::arch::ComponentsIndices::TRANSFORM_COMPONENT);
    ecs::arch::Archetype* cachedAnimationActorsArchetypes[32];
    uint32_t animationActorsArchetypesNumber = 0;
    uint64_t animatedActorsRequiredMask =
        (1ul << ecs::arch::ComponentsIndices::MATERIAL_COMPONENT)
        | (1ul << ecs::arch::ComponentsIndices::ANIMATION_COMPONENT)
        | (1ul << ecs::arch::ComponentsIndices::ROTATION_COMPONENT)
        | (1ul << ecs::arch::ComponentsIndices::TRANSFORM_COMPONENT)
        | (1ul << ecs::arch::ComponentsIndices::MESH_COMPONENT);
    ecs::arch::Archetype* cachedStaticActorsArchetypes[32];
    uint32_t staticActorsArchetypesNumber = 0;
    uint64_t staticActorsRequiredMask =
        (1ul << ecs::arch::ComponentsIndices::MATERIAL_COMPONENT)
        | (1ul << ecs::arch::ComponentsIndices::STATIC_MESH_TAG_COMPONENT)
        | (1ul << ecs::arch::ComponentsIndices::TRANSFORM_COMPONENT)
        | (1ul << ecs::arch::ComponentsIndices::ROTATION_COMPONENT)
        | (1ul << ecs::arch::ComponentsIndices::MESH_COMPONENT);
    ecs::arch::Archetype* cachedPlayerArchetypes[32];
    uint32_t playerArchetypesNumber = 0;
    uint64_t playerRequiredMask =
        (1ul << ecs::arch::ComponentsIndices::PLAYER_TAG_COMPONENT)
        | (1ul << ecs::arch::ComponentsIndices::TRANSFORM_COMPONENT)
        | (1ul << ecs::arch::ComponentsIndices::VIEW_COMPONENT);
    ecs::arch::Archetype* cachedAnimationArchetypes[32];
    uint32_t animationArchetypesNumber = 0;
    uint64_t animationRequiredMask =
        (1ul << ecs::arch::ComponentsIndices::MATERIAL_COMPONENT)
        | (1ul << ecs::arch::ComponentsIndices::ANIMATION_COMPONENT)
        | (1ul << ecs::arch::ComponentsIndices::ROTATION_COMPONENT)
        | (1ul << ecs::arch::ComponentsIndices::TRANSFORM_COMPONENT)
        | (1ul << ecs::arch::ComponentsIndices::MESH_COMPONENT);
    ecs::arch::Archetype* cachedCrosshairActorsArchetypes[32];
    uint32_t crosshairActorsArchetypesNumber = 0;
    uint64_t crosshairRequiredMask =
        (1ul << ecs::arch::ComponentsIndices::CROSSHAIR_TAG_COMPONENT)
        | (1ul << ecs::arch::ComponentsIndices::MESH_COMPONENT)
        | (1ul << ecs::arch::ComponentsIndices::TRANSFORM_COMPONENT);
    ecs::arch::Archetype* cachedLevelChunkActorsArchetypes[32];
    uint32_t levelChunkActorsArchetypesNumber = 0;
    uint64_t levelChunkRequiredMask =
        (1ul << ecs::arch::ComponentsIndices::MATERIAL_COMPONENT)
        | (1ul << ecs::arch::ComponentsIndices::LEVEL_CHUNK_TAG_COMPONENT)
        | (1ul << ecs::arch::ComponentsIndices::TRANSFORM_COMPONENT)
        | (1ul << ecs::arch::ComponentsIndices::ROTATION_COMPONENT)
        | (1ul << ecs::arch::ComponentsIndices::MESH_COMPONENT);
    ecs::arch::Archetype* cachedProjectileActorsArchetypes[32];
    uint32_t projectileActorsArchetypesNumber = 0;
    uint64_t projectileRequiredMask =
        (1ul << ecs::arch::ComponentsIndices::PROJECTILE_BUNDLE_COMPONENT)
        | (1ul << ecs::arch::ComponentsIndices::TRANSFORM_COMPONENT)
        | (1ul << ecs::arch::ComponentsIndices::ROTATION_COMPONENT)
        | (1ul << ecs::arch::ComponentsIndices::MESH_COMPONENT);
    ecs::arch::Archetype* cachedItemActorsArchetypes[32];
    uint32_t itemActorsArchetypesNumber = 0;
    uint64_t rotationItemRequiredMask =
        (1ul << ecs::arch::ComponentsIndices::ITEM_COMPONENT)
        | (1ul << ecs::arch::ComponentsIndices::MESH_COMPONENT)
        | (1ul << ecs::arch::ComponentsIndices::TRANSFORM_COMPONENT)
        | (1ul << ecs::arch::ComponentsIndices::COLLIDER_COMPONENT)
        | (1ul << ecs::arch::ComponentsIndices::COLLIDER_FLAGS_COMPONENT)
        | (1ul << ecs::arch::ComponentsIndices::ROTATION_COMPONENT)
        | (1ul << ecs::arch::ComponentsIndices::MATERIAL_COMPONENT);
    ecs::arch::Archetype* cachedInventoryArchetypes[32];
    uint32_t inventoryArchetypesNumber = 0;
    uint64_t inventoryRequiredMask =
        (1ul << ecs::arch::ComponentsIndices::INVENTORY_COMPONENT)
        | (1ul << ecs::arch::ComponentsIndices::MESH_COMPONENT)
        | (1ul << ecs::arch::ComponentsIndices::TRANSFORM_COMPONENT)
        | (1ul << ecs::arch::ComponentsIndices::MATERIAL_COMPONENT);
    ecs::arch::Archetype* cachedItemArchetypes[32];
    uint32_t itemArchetypesNumber = 0;
    uint64_t itemRequiredMask =
        (1ul << ecs::arch::ComponentsIndices::ITEM_COMPONENT)
        | (1ul << ecs::arch::ComponentsIndices::MESH_COMPONENT)
        | (1ul << ecs::arch::ComponentsIndices::TRANSFORM_COMPONENT)
        | (1ul << ecs::arch::ComponentsIndices::COLLIDER_COMPONENT)
        | (1ul << ecs::arch::ComponentsIndices::COLLIDER_FLAGS_COMPONENT)
        | (1ul << ecs::arch::ComponentsIndices::MATERIAL_COMPONENT);
    ecs::arch::Archetype* cachedHealthBarsArchetypes[32];
    uint32_t healthBarsArchetypesNumber = 0;
    uint64_t healthBarsRequiredMask =
        (1ul << ecs::arch::ComponentsIndices::HEALTH_COMPONENT)
        | (1ul << ecs::arch::ComponentsIndices::MESH_COMPONENT)
        | (1ul << ecs::arch::ComponentsIndices::TRANSFORM_COMPONENT);
    ecs::arch::Archetype* cachedFontsArchetypes[32];
    uint32_t fontsArchetypesNumber = 0;
    uint64_t fontRequiredMask =
        (1ul << ecs::arch::ComponentsIndices::FONT_COMPONENT)
        | (1ul << ecs::arch::ComponentsIndices::TRANSFORM_COMPONENT);
    double fpsAccumulator = 0;
    Engine();

public:
    std::vector<ecs::components::MeshHandle> meshHandlers;
    std::vector<ecs::TextureHandle> textureHandlers;
    uint32_t wavefrontObjCounter = 0;

    ~Engine();

    // Don't need to make copy because of singleton property.
    Engine(Engine& _engine) = delete;
    // Don't need assignment operator because of singleton property.
    void operator=(const Engine& _engine) = delete;
    // It possibly to get only one instance of this class with this method.
    static Engine* GetInstance();
    void GameLoop();
    void EventQueueFlush();
    void RenderVulkan();
    void EnlargeFrameAccumulator(float value);
    void SetViewMatrix();
    void SetProjectionMatrix();
    [[nodiscard]] std::vector<Matrix<float, 4>> updateAnimationFrames(
        ecs::components::animation* animationComponent,
        unsigned int meshID
    );
    Matrix<float, 4> updateDirectionalLightSpaceMatrixShadowMapUBO(
        ecs::components::directionalLight* directionalLightComponent
    );
    Matrix<float, 4> updateSpotLightSpaceMatrixShadowMapUBO(
        ecs::components::spotLight* spotLightComponent
    );
    Matrix<float, 4> updatePointLightSpaceMatrixShadowMapUBO(
        ecs::components::pointLight* pointLightComponent,
        uint32_t layer
    );
    SlotData updateDataUBO_UI(
        const unsigned int currentInventoryRow,
        const unsigned int currentInventoryColumn,
        ecs::components::inventory* inventoryComponent,
        ecs::components::transform* slotTransfromComponent,
        ecs::components::mesh* meshComponent
    );
    Matrix<float, 4> updateDataUBO_IconsUI(
        ecs::components::transform* itemTransfromComponent,
        [[maybe_unused]] ecs::components::collider* itemColliderComponent,
        ecs::components::item* itemComponent,
        const unsigned int rowInventory,
        const unsigned int columnInventory,
        ecs::components::transform* inventoryTransformComponent,
        ecs::components::mesh* itemMesh,
        int itemEntity
    );
    Matrix<float, 4> updateDataHudScreenUBO(
        ecs::components::transform* cursorTransform
    );
    void setFrameData();
    void loadWavefrontObj();
    void calculateMeshBounds(const Vector<float, 4>& animatedVertex);
    bool isModelCacheExists(const std::string& modelFilePath);
    void writeModelsCache(const std::string& modelFilePath);
    void initializeGLTF();
    void initializeFontData();
    Matrix<float, 4> computeModelMatrix(
        ecs::components::transform* _transformComponent,
        ecs::components::rotation* rotation
    );
    void computeHudScreeenCoordinates();
    ecs::TextureHandle LoadTextureFromFile(const char* path_to_texture);
    ecs::TextureHandle LoadTextureFromAddress(
        unsigned int iWidth,
        unsigned int iHeight,
        unsigned int dat_length,
        unsigned char* u_iData
    );
    ecs::components::MeshHandle LoadMeshFromFile_OBJ(const char* _pathToMesh);
    ecs::components::MeshHandle LoadMeshFromFile_GLTF(const char* pathToMesh);
    ecs::components::MeshHandle LoadMesh();
    void GameKill();
};
} // namespace glvm::core
