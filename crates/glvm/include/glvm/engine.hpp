// This file is part of Game Loop Versatile Modules (GLVM)
// Copyright © 2024 Maksim Manokhin a.k.a. Yuriorkis_Scream. Contacts:
// <fellfrostqtw@gmail.com> Author: Maksim Manokhin a.k.a. Yuriorkis_Scream
// License: http://opensource.org/licenses/MIT
#pragma once

#include "glvm/component_manager.hpp"
#include "glvm/components_full_set.hpp"
#include "glvm/constants.hpp"
#include "glvm/entity_manager.hpp"
#include "glvm/event.hpp"
#include "glvm/events_stack.hpp"
#include "glvm/graphic_api/vulkan.hpp"
#include "glvm/i_chrono.hpp"
#include "glvm/i_container.hpp"
#include "glvm/i_sound_engine.hpp"
#include "glvm/i_window.hpp"
#include "glvm/system_manager.hpp"
#include "glvm/systems_full_set.hpp"
#include "glvm/texture.hpp"
#include "glvm/texture_manager.hpp"
#include "glvm/timer_creator.hpp"
#include "glvm/vector.hpp"

#include <mutex>

using Entity = unsigned int;

namespace GLVM::core {
enum RendererType { VULKAN_RENDERER };

class Engine {
    static Engine* pInstance_;
    static std::mutex Mutex_;

    Time::IChrono* chrono;
    Sound::ISoundEngine* soundEngine;

    float deltaFrameTime;
    float gravity;
    CStack Input_Stack_;
    std::vector<ecs::Texture> textureVector;
    core::vector<ecs::TextureHandle> textureHandlers;
    std::vector<const char*> pathsArray_;
    core::vector<const char*> pathsGLTF_;
    uint32_t meshID = 0;
    core::vector<ecs::components::MeshHandle> meshHandlers;
    CVulkanRenderer* vulkanRenderer;

    ecs::CCollisionSystem* collisionSystem;
    ecs::CMovementSystem* movementSystem;
    ecs::CPhysicsSystem* physicsSystem;
    ecs::CProjectileSystem* projectileSystem;

    // For FPS counting
    unsigned int fpsCounter = 0;
    double fpsAccumulator = 0;

    Engine();

public:
    ~Engine();

    // Dont need to make copy because of singleton property.
    Engine(Engine& _engine) = delete;

    // Dont need assignment operator because of singleton property.
    void operator=(const Engine& _engine) = delete;

    // It possibly to get only one instance of this class with this method.
    static Engine* GetInstance();

    void GameLoop(RendererType renderer);
    void EventQueueFlush();
    void RenderVulkan();
    ecs::TextureHandle LoadTextureFromFile(const char* path_to_texture);
    ecs::TextureHandle LoadTextureFromAddress(
        unsigned int iWidth,
        unsigned int iHeight,
        unsigned int dat_length,
        unsigned char* u_iData
    );
    ecs::components::MeshHandle LoadMeshFromFile_OBJ(const char* _pathToMesh);
    ecs::components::MeshHandle LoadMeshFromFile_GLTF(const char* pathToMesh);
    void FPScounter();
    void GameKill();
};
} // namespace GLVM::core
