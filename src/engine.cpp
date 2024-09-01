// This file is part of Game Loop Versatile Modules (GLVM)
// Copyright © 2024 Maksim Manokhin a.k.a. Yuriorkis_Scream. Contacts:
// <fellfrostqtw@gmail.com> Author: Maksim Manokhin a.k.a. Yuriorkis_Scream
// License: http://opensource.org/licenses/MIT

#include "engine.hpp"
#include "components/vertex_component.hpp"
#include "graphic_api/vulkan.hpp"
#include "i_sound_engine.hpp"
#include "sound_engine_factory.hpp"
#include "system_manager.hpp"
#include "systems/camera_system.hpp"
#include "systems/collision_system.hpp"
#include "systems/movement_system.hpp"
#include "systems/physics_system.hpp"
#include "systems/projectile_system.hpp"
#include "texture.hpp"

#include <cstdint>
#include <limits>
#include <mutex>
#include <sys/types.h>
#include <thread>

GLVM::core::CEvent g_eEvent;

namespace GLVM::core {
Engine *Engine::pInstance_ = nullptr;
std::mutex Engine::Mutex_;

void PlaybackSound(Sound::ISoundEngine *_sound_Engine) {
    while (1) {
        _sound_Engine->SoundStream();
    }
}

Engine::Engine() {
    chrono = Time::CTimerCreator().Create();
    soundEngine = Sound::CSoundEngineFactory().CreateSoundEngine();

    collisionSystem = new ecs::CCollisionSystem(Input_Stack_);
    movementSystem = new ecs::CMovementSystem(Input_Stack_);
    physicsSystem = new ecs::CPhysicsSystem(gravity, Input_Stack_);
    projectileSystem = new ecs::CProjectileSystem(Input_Stack_);

    deltaFrameTime = 0.0;
    g_eEvent.SetEvent(eDEFAULT);

    ecs::CSystemManager *pSystem_Manager = ecs::CSystemManager::GetInstance();

    // Call of ActivateSystem function must be in this order.
    pSystem_Manager->ActivateSystem(movementSystem);
    pSystem_Manager->ActivateSystem(projectileSystem);
    pSystem_Manager->ActivateSystem(collisionSystem);
    pSystem_Manager->ActivateSystem(physicsSystem);

    std::thread sound_thread(PlaybackSound, std::ref(soundEngine));
    sound_thread.detach();
}

Engine::~Engine() {
}

Engine *Engine::GetInstance() {
    std::lock_guard<std::mutex> lock(Mutex_);
    if (pInstance_ == nullptr) {
        pInstance_ = new Engine();
    }
    return pInstance_;
}

void Engine::GameLoop(RendererType renderer) {
    if (renderer == VULKAN_RENDERER) {
        RenderVulkan();
        return;
    }
}

void Engine::EventQueueFlush() {
}

void Engine::RenderVulkan() {
    ecs::CSystemManager *pSystem_Manager = ecs::CSystemManager::GetInstance();
    bool bGame_Loop_Active = true;

    projectileSystem->textureHandlers = textureHandlers;
    projectileSystem->meshHandlers = meshHandlers;

    vulkanRenderer = new CVulkanRenderer();
    vulkanRenderer->initializeTextureData_ = textureVector;
    vulkanRenderer->pathsArray_ = pathsArray_;
    vulkanRenderer->pathsGLTF_ = pathsGLTF_;
    vulkanRenderer->run();
    vulkanRenderer->Window.Input_Stack_ = &Input_Stack_;

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

        vulkanRenderer->Window.ClearDisplay();

        vulkanRenderer->Window.HandleEvent(g_eEvent);
        if ((Input_Stack_.SearchElement(EEvents::eGAME_LOOP_KILL)) ==
            EEvents::eGAME_LOOP_KILL) {
            bGame_Loop_Active = false;
        }
        g_eEvent.SetLastEvent(Input_Stack_);

        vulkanRenderer->Window.CursorLock(
                g_eEvent.mousePointerPosition.position_X,
                g_eEvent.mousePointerPosition.position_Y,
                &g_eEvent.mousePointerPosition.offset_X,
                &g_eEvent.mousePointerPosition.offset_Y);

        movementSystem->deltaFrameTime = deltaFrameTime;
        movementSystem->gravity = gravity;
        collisionSystem->fDelta_Time_ = deltaFrameTime;
        collisionSystem->gravity = gravity;
        projectileSystem->deltaFrameTime = deltaFrameTime;
        projectileSystem->soundEngine = soundEngine;
        physicsSystem->fDelta_Time_ = deltaFrameTime;
        physicsSystem->fAcceleration_of_Gravity_ += (deltaFrameTime / 20);
        physicsSystem->gravity = gravity;
        vulkanRenderer->EnlargeFrameAccumulator(deltaFrameTime);
        pSystem_Manager->Update();
        vulkanRenderer->draw();
        vulkanRenderer->Window.SwapBuffers();
    }

    vulkanRenderer->Window.Close();
}

ecs::TextureHandle Engine::LoadTextureFromFile(const char *path_to_texture) {
    uint32_t textureID = textureVector.size();
    ecs::TextureHandle textureHandle;
    textureHandle.id = textureID;
    textureVector.push_back({.path_to_image = path_to_texture});
    textureHandlers.Push(textureHandle);

    return textureHandle;
}

ecs::TextureHandle Engine::LoadTextureFromAddress(unsigned int iWidth,
                                                  unsigned int iHeight,
                                                  unsigned int dat_length,
                                                  unsigned char *u_iData) {
    uint32_t textureID = textureVector.size();
    ecs::TextureHandle textureHandle;
    textureHandle.id = textureID;
    textureVector.push_back({.iWidth_ = iWidth,
                             .iHeight_ = iHeight,
                             .dat_length_ = dat_length,
                             .u_iData_ = u_iData});
    textureHandlers.Push(textureHandle);

    return textureHandle;
}

ecs::components::MeshHandle
Engine::LoadMeshFromFile_OBJ(const char *_pathToMesh) {
    ecs::components::MeshHandle meshHandle;
    meshHandle.id = meshID;
    pathsArray_.push_back(_pathToMesh);
    meshHandlers.Push(meshHandle);
    ++meshID;

    return meshHandle;
}

ecs::components::MeshHandle
Engine::LoadMeshFromFile_GLTF(const char *pathToMesh) {
    ecs::components::MeshHandle meshHandle;
    meshHandle.id = meshID;
    pathsGLTF_.Push(pathToMesh);
    meshHandlers.Push(meshHandle);
    ++meshID;

    return meshHandle;
}

void Engine::FPScounter() {
    ++fpsCounter;
    fpsAccumulator += deltaFrameTime;
    if (fpsAccumulator > 1.0f) {
        fpsCounter = 0;
        fpsAccumulator = 0;
    }
}

void Engine::GameKill() {
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
}
} // namespace GLVM::core
