#include "glvm/Engine.hpp"

#include "glvm/ArchetypeECS/ArchECS_Types.hpp"
#include "glvm/ArchetypeECS/ArchECS_Utils.hpp"
#include "glvm/ArchetypeECS/ArchetypeInterface.hpp"
#include "glvm/Archetypes/CrosshairArchetype.hpp"
#include "glvm/Archetypes/DirectionalLightArchetype.hpp"
#include "glvm/Archetypes/EnemyArchetype.hpp"
#include "glvm/Archetypes/InventoryArchetype.hpp"
#include "glvm/Archetypes/ItemArchetype.hpp"
#include "glvm/Archetypes/LevelChunkArchetype.hpp"
#include "glvm/Archetypes/PlayerArchetype.hpp"
#include "glvm/Archetypes/ProjectileArchetype.hpp"
#include "glvm/Archetypes/StaticMeshArchetype.hpp"
#include "glvm/Common/CommonFunctions.hpp"
#include "glvm/Components/AnimationComponent.hpp"
#include "glvm/Components/ColliderComponent.hpp"
#include "glvm/Components/DirectionalLightComponent.hpp"
#include "glvm/Components/HealthComponent.hpp"
#include "glvm/Components/InventoryComponent.hpp"
#include "glvm/Components/MaterialComponent.hpp"
#include "glvm/Components/PointLightComponent.hpp"
#include "glvm/Components/ProjectileBundle.hpp"
#include "glvm/Components/RotationComponent.hpp"
#include "glvm/Components/TransformComponent.hpp"
#include "glvm/Components/VertexComponent.hpp"
#include "glvm/Components/ViewComponent.hpp"
#include "glvm/Event.hpp"
#include "glvm/EventsStack.hpp"
#include "glvm/GraphicAPI/Vulkan.hpp"
#include "glvm/ISoundEngine.hpp"
#include "glvm/ProceduralLevelGeneratingSystem.hpp"
#include "glvm/ShaderStructs.hpp"
#include "glvm/SoundEngineFactory.hpp"
#include "glvm/SystemManager.hpp"
#include "glvm/Systems/CollisionSystem.hpp"
#include "glvm/Systems/DamageSystem.hpp"
#include "glvm/Systems/EnemySystem.hpp"
#include "glvm/Systems/InventorySystem.hpp"
#include "glvm/Systems/ItemSystem.hpp"
#include "glvm/Systems/MovementSystem.hpp"
#include "glvm/Systems/PhysicsSystem.hpp"
#include "glvm/Systems/ProjectileSystem.hpp"
#include "glvm/TagComponents/LevelChunkTagComponent.hpp"
#include "glvm/Texture.hpp"
#include "glvm/VkStructs.hpp"

#include <cstdint>
#include <limits>
#include <mutex>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <thread>

glvm::core::CStack Input_Stack_ {};

int x_pointer;
int y_pointer;

#ifdef __linux__
#include <wayland-client-core.h>
#endif
#include <fstream>

glvm::core::CEvent g_eEvent;
// Contains all maximum absolute axis values.
glvm::core::vector<glvm::core::MeshAxisMaxAbsoluteValues>
    allMeshMaxAbsoluteValues;

namespace glvm::core {
Engine* Engine::pInstance_ = nullptr;
std::mutex Engine::Mutex_;

void PlaybackSound(
    Sound::ISoundEngine* _sound_Engine,
    std::atomic<bool>& runningSound
) {
    runningSound = true;
    while (runningSound) {
        _sound_Engine->SoundStream();
    }
}

Engine::Engine() {
    chrono = Time::CTimerCreator().Create();
    soundEngine = Sound::CSoundEngineFactory().CreateSoundEngine();

    spatialGridSystem = new ecs::SpatialGridSystem();
    collisionSystem = new ecs::CCollisionSystem(Input_Stack_);
    movementSystem = new ecs::CMovementSystem(Input_Stack_);
    physicsSystem = new ecs::CPhysicsSystem(gravity, Input_Stack_);
    projectileSystem = new ecs::CProjectileSystem(Input_Stack_);
    damageSystem = new ecs::DamageSystem();
    enemySytem = new ecs::EnemySystem();
    itemSystem = new ecs::ItemSystem();
    procuduralLevelGeneratingSystem = new ProceduralLevelGeneratingSystem();
    inventorySystem = new ecs::InventorySystem();

    deltaFrameTime = 0.0;
    g_eEvent.SetEvent(eDEFAULT);

    ecs::CSystemManager* pSystem_Manager = ecs::CSystemManager::GetInstance();

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

Engine* Engine::GetInstance() {
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
    namespace arch = glvm::ecs::arch;
    std::cout << "INNER CALL ARCHETYPES NUMBER: "
              << arch::world.archetypes.GetSize() << std::endl;
    ecs::CSystemManager* pSystem_Manager = ecs::CSystemManager::GetInstance();
    bool bGame_Loop_Active = true;

    projectileSystem->textureHandlers = textureHandlers;
    projectileSystem->meshHandlers = meshHandlers;

    enemySytem->textureHandlers = textureHandlers;
    enemySytem->meshHandlers = meshHandlers;

    procuduralLevelGeneratingSystem->meshHandlers = meshHandlers;
    procuduralLevelGeneratingSystem->textureHandlers = textureHandlers;

    inventorySystem->isItemDraged = &dragedItemEntity;
    itemSystem->dragedItemEntity = &dragedItemEntity;

    vulkanRenderer = new CVulkanRenderer();
    vulkanRenderer->initializeTextureData_ = textureVector;
    vulkanRenderer->pathsArray_ = pathsArray_;
    vulkanRenderer->pathsGLTF_ = pathsGLTF_;
    glvm::core::MeshManager* meshManager =
        glvm::core::MeshManager::GetInstance();
    vulkanRenderer->SetMeshData(
        meshManager->pathsArray_,
        meshManager->pathsGLTF_
    );
    namespace cm = glvm::ecs::components;

    directionalLightArchetypesNumber = 0;
    arch::world.searchCacheArchetypes(
        directionalLightRequiredMask,
        cachedDirectionalLigthArchetypes,
        directionalLightArchetypesNumber
    );
    vulkanRenderer->directionalLightNumber = directionalLightArchetypesNumber;

    spotLightArchetypesNumber = 0;
    arch::world.searchCacheArchetypes(
        spotLightRequiredMask,
        cachedSpotLigthArchetypes,
        spotLightArchetypesNumber
    );
    vulkanRenderer->spotLightNumber = spotLightArchetypesNumber;

    pointLightArchetypesNumber = 0;
    arch::world.searchCacheArchetypes(
        pointLightRequiredMask,
        cachedPointLigthArchetypes,
        pointLightArchetypesNumber
    );
    vulkanRenderer->pointLightNumber = pointLightArchetypesNumber;

    animationActorsArchetypesNumber = 0;
    arch::world.searchCacheArchetypes(
        animatedActorsRequiredMask,
        cachedAnimationActorsArchetypes,
        animationActorsArchetypesNumber
    );

    staticActorsArchetypesNumber = 0;
    arch::world.searchCacheArchetypes(
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
                    ecs::DeactivatedSystems::DEACTIVATED_MOVEMENT_SYSTEM
                );
                hud_screen_x = 0.0f;
                hud_screen_y = 0.0f;
            } else {
                pSystem_Manager->ReturnSystemToActivatedState(
                    ecs::DeactivatedSystems::DEACTIVATED_MOVEMENT_SYSTEM
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
        vulkanRenderer->dragedItemEntity = dragedItemEntity;
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
    namespace cm = glvm::ecs::components;
    namespace arch = glvm::ecs::arch;
    animationArchetypesNumber = 0;
    for (uint32_t m = 0; m < arch::world.archetypes.GetSize(); ++m) {
        arch::Archetype* arch = arch::world.archetypes[m];
        arch::componentMask requiredMask =
            (1ul << arch::ComponentsIndices::MESH_COMPONENT)
            | (1ul << arch::ComponentsIndices::ANIMATION_COMPONENT);

        if (arch::matchesRequiredMask(arch->mask, requiredMask)) {
            cachedAnimationArchetypes[animationArchetypesNumber] = arch;
            ++animationArchetypesNumber;
        }
    }

    for (uint32_t n = 0; n < animationArchetypesNumber; ++n) {
        arch::Archetype* arch = cachedAnimationArchetypes[n];
        cm::animation* animationView = nullptr;
        cm::mesh* meshView = nullptr;
        if (arch != nullptr) {
            switch (arch->mask) {
                case arch::enemyComponentMask:
                    animationView =
                        static_cast<arch::EnemyArchetype*>(arch)->animations;
                    meshView = static_cast<arch::EnemyArchetype*>(arch)->meshes;
                    break;
                case arch::playerComponentMask:
                    animationView =
                        static_cast<arch::PlayerArchetype*>(arch)->animations;
                    meshView =
                        static_cast<arch::PlayerArchetype*>(arch)->meshes;
                    break;
            }

            for (unsigned int i = 0;
                 i < cachedAnimationArchetypes[n]->entityCount;
                 ++i) {
                if (&meshView[i] != nullptr && &animationView[i] != nullptr) {
                    unsigned int mesh_id = meshView[i].handle.id;
                    if (vulkanRenderer->jointMatricesPerMesh.GetSize() > 0
                        && vulkanRenderer->jointMatricesPerMesh[mesh_id]
                                .GetSize()
                            > 0) {
                        animationView[i].frameAccumulator += value;
                    }
                }
            }
        }
    }
}

void Engine::SetViewMatrix() {
    namespace cm = glvm::ecs::components;
    namespace arch = glvm::ecs::arch;

    playerArchetypesNumber = 0;
    arch::world.searchCacheArchetypes(
        playerRequiredMask,
        cachedPlayerArchetypes,
        playerArchetypesNumber
    );

    for (uint32_t n = 0; n < playerArchetypesNumber; ++n) {
        arch::Archetype* arch = cachedPlayerArchetypes[n];
        cm::beholder* views =
            (ecs::components::beholder*)
                arch->components[arch::ComponentsIndices::VIEW_COMPONENT];
        cm::transform* transfroms =
            (ecs::components::transform*)
                arch->components[arch::ComponentsIndices::TRANSFORM_COMPONENT];

        for (uint32_t x = 0; x < arch->entityCount; ++x) {
            cm::beholder* cameraComponent = &views[x];
            cm::transform* _Player = &transfroms[x];

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

            const vec3 rightVec =
                Cross(cameraComponent->forward, vec3(0.0f, -1.0f, 0.0));
            const vec3 newUpVec = Cross(rightVec, cameraComponent->forward);
            // 1. The mouse direction determines the "intended direction of
            // rotation" for the object.
            // 2. The camera is "looking forward."
            // 3. To make the object "rotate as if the mouse is pushing it," you
            // need to rotate it around an axis that is perpendicular to both
            // the view direction and the mouse movement.
            const vec3 rotateAxis = Normalize(Cross(
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
                pga::point appliedRotationPoint =
                    exp(rotationAngle,
                        pga::rline {
                            .rx = -rotateAxis.m_vector[0],
                            .ry = -rotateAxis.m_vector[1],
                            .rz = -rotateAxis.m_vector[2]
                        })
                    >> pga::point {
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
            mat4 view = LookAtMain(
                cameraComponent->Position + _Player->position,
                cameraComponent->Position + _Player->position
                    + cameraComponent->forward,
                vec3(0.0f, -1.0f, 0.0)
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
    mat4 tProjection_Matrix =
        Perspective(Radians(90.0f), vulkanRenderer->aspectRate, 0.1f, 100.0f);
    vulkanRenderer->projectionMatrix = tProjection_Matrix;
    vulkanRenderer->projectionMatrix[1][1] *= 1.0f;
}

[[nodiscard]] core::vector<mat4> Engine::updateAnimationFrames(
    [[maybe_unused]] ecs::components::animation* animationComponent,
    [[maybe_unused]] unsigned int meshID
) {
    if (vulkanRenderer->jointMatricesPerMesh.GetSize() > 0
        && vulkanRenderer->jointMatricesPerMesh[meshID].GetSize() > 0
        && animationComponent->frameAccumulator
            >= vulkanRenderer
                    ->frames[meshID][animationComponent->currentAnimationFrame]
                * 1.0f) {
        ++animationComponent->currentAnimationFrame;
        if (vulkanRenderer->jointMatricesPerMesh[meshID].GetSize() > 0
            && animationComponent->currentAnimationFrame
                == vulkanRenderer->frames[meshID].GetSize()) {
            animationComponent->currentAnimationFrame = 0;
            animationComponent->frameAccumulator = 0.0f;
        }
    }

    unsigned int joinMatricesDataSize {};
    if (vulkanRenderer->jointMatricesPerMesh.GetSize() > 0) {
        joinMatricesDataSize =
            vulkanRenderer->jointMatricesPerMesh[meshID].GetSize();
    }

    core::vector<mat4> jointMatrices;
    if (joinMatricesDataSize == 0) {
        jointMatrices.Resize(MAX_JOINTS_NUMBER);
        for (unsigned int i = 0; i < MAX_JOINTS_NUMBER; ++i) {
            mat4 unitMatrix(1.0f);
            jointMatrices[i] = unitMatrix;
        }

    } else {
        jointMatrices.Resize(MAX_JOINTS_NUMBER);
        for (unsigned int i = 0; i < joinMatricesDataSize; ++i) {
            if (meshID >= vulkanRenderer->jointMatricesPerMesh.GetSize()) {
                std::cout << "OUTER ARRAY OVERFLOW" << std::endl;
                throw("sdfsdf");
            } else if (
                i >= vulkanRenderer->jointMatricesPerMesh[meshID].GetSize()
            ) {
                std::cout << "MIDDLE ARRAY OVERFLOW" << std::endl;
                throw("sdfsdf");
            } else if (
                animationComponent->currentAnimationFrame
                >= vulkanRenderer->jointMatricesPerMesh[meshID][i].GetSize()
            ) {
                std::cout << "frame: "
                          << animationComponent->currentAnimationFrame
                          << std::endl;
                std::cout
                    << "array size: "
                    << vulkanRenderer->jointMatricesPerMesh[meshID][i].GetSize()
                    << std::endl;
                std::cout << "frames number: "
                          << vulkanRenderer->frames[meshID].GetSize()
                          << std::endl;
                std::cout << "INNER ARRAY OVERFLOW" << std::endl;
                throw("sdfsdf");
            }

            jointMatrices[i] =
                vulkanRenderer->jointMatricesPerMesh
                    [meshID][i][animationComponent->currentAnimationFrame];
        }

        for (u32 j = joinMatricesDataSize; j < MAX_JOINTS_NUMBER; ++j) {
            mat4 unitMatrix(1.0f);
            jointMatrices[j] = unitMatrix;
        }
    }

    return jointMatrices;
}

mat4 Engine::updateDirectionalLightSpaceMatrixShadowMapUBO(
    ecs::components::directionalLight* directionalLightComponent
) {
    float nearPlaneFlatShadowMap = 5.5f;
    float farPlaneFlatShadowMap = 100.0f;
    mat4 directionalProjectionMatrixLight = ortho(
        -50.0f,
        50.0f,
        -50.0f,
        50.0f,
        nearPlaneFlatShadowMap,
        farPlaneFlatShadowMap
    );

    vec3 positionVectorLight = directionalLightComponent->position;
    vec3 directionVectorLight = directionalLightComponent->direction;

    mat4 viewMatrixLight = LookAtMain(
        positionVectorLight,
        directionVectorLight,
        {0.0f, -1.0f, 0.0f}
    );
    return viewMatrixLight * directionalProjectionMatrixLight;
}

mat4 Engine::updateSpotLightSpaceMatrixShadowMapUBO(
    ecs::components::spotLight* spotLightComponent
) {
    float nearPlaneFlatShadowMap = 0.5f;
    float farPlaneFlatShadowMap = 100.0f;
    mat4 spotProjectionMatrixLight = Perspective(
        Radians(90.0f),
        (float)SHADOW_MAP_SIZE / (float)SHADOW_MAP_SIZE,
        nearPlaneFlatShadowMap,
        farPlaneFlatShadowMap
    );

    vec3 positionVectorLight = spotLightComponent->position;
    vec3 directionVectorLight = spotLightComponent->direction;
    mat4 viewMatrixLight = LookAtMain(
        positionVectorLight,
        directionVectorLight,
        {0.0f, -1.0f, 0.0f}
    );
    return viewMatrixLight * spotProjectionMatrixLight;
}

mat4 Engine::updatePointLightSpaceMatrixShadowMapUBO(
    ecs::components::pointLight* pointLightComponent,
    uint32_t layer
) {
    vec3 positionVectorLight = pointLightComponent->position;
    vec3 directionalVectorLight = vec3(0.0f, 0.0f, 0.0f);
    vec3 upVector = {0.0, 0.0, 0.0};

    switch (layer) {
        case 0:
            // Positive X.
            directionalVectorLight =
                positionVectorLight + vec3(1.0f, 0.0f, 0.0f);
            upVector = vec3(0.0f, -1.0f, 0.0f);
            break;
        case 1:
            // Negative X.
            directionalVectorLight =
                positionVectorLight + vec3(-1.0f, 0.0f, 0.0f);
            upVector = vec3(0.0f, -1.0f, 0.0f);
            break;
        case 2:
            // Positive Y.
            directionalVectorLight =
                positionVectorLight + vec3(0.0f, 1.0f, 0.0f);
            upVector = vec3(0.0f, 0.0f, 1.0f);
            break;
        case 3:
            // Negative Y.
            directionalVectorLight =
                positionVectorLight + vec3(0.0f, -1.0f, 0.0f);
            upVector = vec3(0.0f, 0.0f, -1.0f);
            break;
        case 4:
            // Positive Z.
            directionalVectorLight =
                positionVectorLight + vec3(0.0f, 0.0f, 1.0f);
            upVector = vec3(0.0f, -1.0f, 0.0f);
            break;
            // Negative Z.
        case 5:
            directionalVectorLight =
                positionVectorLight + vec3(0.0f, 0.0f, -1.0f);
            upVector = vec3(0.0f, -1.0f, 0.0f);
            break;
        default:
            break;
    }

    mat4 projectionMatrixCubeShadowMap = Perspective(
        Radians(90.0f),
        (float)SHADOW_MAP_SIZE / (float)SHADOW_MAP_SIZE,
        0.3f,
        100.0f
    );

    mat4 viewMatrixLight =
        LookAtMain(positionVectorLight, directionalVectorLight, upVector);

    return viewMatrixLight * projectionMatrixCubeShadowMap;
}

[[nodiscard]] SlotData Engine::updateDataUBO_UI(
    const unsigned int currentInventoryRow,
    const unsigned int currentInventoryColumn,
    ecs::components::inventory* inventoryComponent,
    ecs::components::transform* slotTransfromComponent,
    ecs::components::mesh* meshComponent
) {
    SlotData hudUBO {};
    mat4 model(1.0);
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
    for (unsigned int i = 0; i < inventoryComponent->highlightedSlots.GetSize();
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

    if (inventoryComponent->highlightedSlots.GetSize() > 0) {
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

mat4 Engine::updateDataUBO_IconsUI(
    ecs::components::transform* itemTransfromComponent,
    [[maybe_unused]] ecs::components::collider* itemColliderComponent,
    ecs::components::item* itemComponent,
    const unsigned int rowInventory,
    const unsigned int columnInventory,
    ecs::components::transform* inventoryTransformComponent,
    ecs::components::mesh* itemMesh,
    int itemEntity
) {
    float x_result_offset = 0.0f;
    float y_result_offset = 0.0f;
    if (itemComponent->occupiedSlots.GetSize() == 0) {
    } else {
        const unsigned int inventorySlotEntity_0 =
            itemComponent->occupiedSlots[0];
        const unsigned int inventorySlotEntity_3 =
            itemComponent->occupiedSlots.GetHead();
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

    if (dragedItemEntity != itemEntity) {
        itemTransfromComponent->position =
            vec3(x_result_offset, y_result_offset, 0.1f);
    } else {
        itemScale *= 1.1f;
        itemTransfromComponent->position[2] = 0.0f;
    }
    mat4 model(1.0);
    model[0][0] = itemScale * itemComponent->itemSlotType.width;
    model[1][1] = itemScale * itemComponent->itemSlotType.height;
    model[2][2] = 0.0f;
    model[3][0] = itemTransfromComponent->position[0];
    model[3][1] = itemTransfromComponent->position[1];
    model[3][2] = itemTransfromComponent->position[2];

    return model;
}

mat4 Engine::updateDataHudScreenUBO(
    ecs::components::transform* cursorTransform
) {
    mat4 model;
    vec3 defaultPosition = vec3(0.0, 0.0, 0.0);

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
    namespace cm = glvm::ecs::components;
    namespace arch = glvm::ecs::arch;

    vulkanRenderer->directionalLights.clear();
    directionalLightArchetypesNumber = 0;
    arch::world.searchCacheArchetypes(
        directionalLightRequiredMask,
        cachedDirectionalLigthArchetypes,
        directionalLightArchetypesNumber
    );

    uint32_t directionalLightCounter = 0;
    for (uint32_t x = 0; x < directionalLightArchetypesNumber; ++x) {
        arch::Archetype* arch = cachedDirectionalLigthArchetypes[x];
        cm::directionalLight* directionalLights =
            (ecs::components::directionalLight*)arch->components
                [arch::ComponentsIndices::DIRECTIONAL_LIGHT_COMPONENT];

        for (uint32_t x1 = 0; x1 < arch->entityCount; ++x1) {
            if (directionalLights) {
                vulkanRenderer->directionalLights.Push({});
                cm::directionalLight* directionalLightComponent =
                    &directionalLights[x1];
                vulkanRenderer->directionalLights[directionalLightCounter]
                    .DirectionalLightSpaceMatrix =
                    updateDirectionalLightSpaceMatrixShadowMapUBO(
                        directionalLightComponent
                    );
                vulkanRenderer->directionalLights[directionalLightCounter]
                    .position = vec4(
                    directionalLightComponent->position[0],
                    directionalLightComponent->position[1],
                    directionalLightComponent->position[2],
                    0.0
                );
                vulkanRenderer->directionalLights[directionalLightCounter]
                    .direction = vec4(
                    directionalLightComponent->direction[0],
                    directionalLightComponent->direction[1],
                    directionalLightComponent->direction[2],
                    0.0
                );
                vulkanRenderer->directionalLights[directionalLightCounter]
                    .ambient = vec4(
                    directionalLightComponent->ambient[0],
                    directionalLightComponent->ambient[1],
                    directionalLightComponent->ambient[2],
                    0.0
                );
                vulkanRenderer->directionalLights[directionalLightCounter]
                    .diffuse = vec4(
                    directionalLightComponent->diffuse[0],
                    directionalLightComponent->diffuse[1],
                    directionalLightComponent->diffuse[2],
                    0.0
                );
                vulkanRenderer->directionalLights[directionalLightCounter]
                    .specular = vec4(
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
    arch::world.searchCacheArchetypes(
        spotLightRequiredMask,
        cachedSpotLigthArchetypes,
        spotLightArchetypesNumber
    );

    uint32_t spotLightCounter = 0;
    for (uint32_t x = 0; x < spotLightArchetypesNumber; ++x) {
        arch::Archetype* arch = cachedSpotLigthArchetypes[x];
        cm::spotLight* spotLights =
            (ecs::components::spotLight*)
                arch->components[arch::ComponentsIndices::SPOT_LIGHT_COMPONENT];

        for (uint32_t x1 = 0; x1 < arch->entityCount; ++x1) {
            if (spotLights) {
                vulkanRenderer->spotLights.Push({});
                cm::spotLight* spotLightComponent = &spotLights[x1];
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
    arch::world.searchCacheArchetypes(
        pointLightRequiredMask,
        cachedPointLigthArchetypes,
        pointLightArchetypesNumber
    );

    uint32_t pointLightCounter = 0;
    for (uint32_t x = 0; x < pointLightArchetypesNumber; ++x) {
        arch::Archetype* arch = cachedPointLigthArchetypes[x];
        cm::pointLight* pointLights =
            (ecs::components::pointLight*)arch
                ->components[arch::ComponentsIndices::POINT_LIGHT_COMPONENT];

        for (uint32_t x1 = 0; x1 < arch->entityCount; ++x1) {
            if (pointLights) {
                vulkanRenderer->pointLights.Push({});
                cm::pointLight* pointLightComponent = &pointLights[x1];
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
    arch::world.searchCacheArchetypes(
        healthBarsRequiredMask,
        cachedHealthBarsArchetypes,
        healthBarsArchetypesNumber
    );

    uint32_t healthBarCounter = 0;
    for (uint32_t x = 0; x < healthBarsArchetypesNumber; ++x) {
        arch::Archetype* arch = cachedHealthBarsArchetypes[x];
        cm::transform* healthBarTransforms =
            (ecs::components::transform*)
                arch->components[arch::ComponentsIndices::TRANSFORM_COMPONENT];
        cm::mesh* healthBarMeshes =
            (ecs::components::mesh*)
                arch->components[arch::ComponentsIndices::MESH_COMPONENT];
        cm::health* healthBars =
            (ecs::components::health*)
                arch->components[arch::ComponentsIndices::HEALTH_COMPONENT];

        unsigned int uiVertexId = 0;
        if (ecs::arch::matchesRequiredMask(
                arch->mask,
                arch::playerComponentMask
            )) {
            uiVertexId = healthBarMeshes[0].handle.id;
        }

        for (unsigned int i = 0; i < arch->entityCount; ++i) {
            vulkanRenderer->healthBars.Push({});
            cm::transform* transformComponent = &healthBarTransforms[i];
            cm::health* healthComponent = &healthBars[i];
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
    arch::world.searchCacheArchetypes(
        fontRequiredMask,
        cachedFontsArchetypes,
        fontsArchetypesNumber
    );

    uint32_t fontCounter = 0;
    for (uint32_t x = 0; x < fontsArchetypesNumber; ++x) {
        arch::Archetype* arch = cachedFontsArchetypes[x];
        cm::transform* fontTransforms =
            (ecs::components::transform*)
                arch->components[arch::ComponentsIndices::TRANSFORM_COMPONENT];
        cm::font* fonts =
            (ecs::components::font*)
                arch->components[arch::ComponentsIndices::FONT_COMPONENT];

        for (unsigned int i = 0; i < arch->entityCount; ++i) {
            vulkanRenderer->fonts.Push({});
            cm::font* fontComponent = &fonts[i];
            cm::transform* transformComponent = &fontTransforms[i];
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
        arch::world.searchCacheArchetypes(
            inventoryRequiredMask,
            cachedInventoryArchetypes,
            inventoryArchetypesNumber
        );

        for (uint32_t x = 0; x < inventoryArchetypesNumber; ++x) {
            arch::Archetype* arch = cachedInventoryArchetypes[x];
            cm::transform* inventoryTransforms =
                (ecs::components::transform*)arch
                    ->components[arch::ComponentsIndices::TRANSFORM_COMPONENT];
            cm::inventory* inventory =
                (ecs::components::inventory*)arch
                    ->components[arch::ComponentsIndices::INVENTORY_COMPONENT];
            cm::material* inventoryMaterials =
                (ecs::components::material*)arch
                    ->components[arch::ComponentsIndices::MATERIAL_COMPONENT];
            cm::mesh* inventoryMeshes =
                (ecs::components::mesh*)
                    arch->components[arch::ComponentsIndices::MESH_COMPONENT];

            if (inventoryTransforms && inventoryMaterials && inventory
                && inventoryMeshes) {
                for (unsigned int i = 0; i < arch->entityCount; ++i) {
                    vulkanRenderer->inventories.Push({});
                    cm::inventory* inventoryComponent = &inventory[i];
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
                            cm::transform* slotTransformComponent =
                                &inventoryTransforms[i];
                            vulkanRenderer->inventories[inventoryCounter]
                                .slotData.Push({});
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
                    cm::inventory* inventoryComponent = &inventory[i];
                    cm::transform* inventoryTransformComponent =
                        &inventoryTransforms[i];

                    vulkanRenderer->items.clear();
                    uint32_t itemCounter = 0;
                    itemArchetypesNumber = 0;
                    arch::world.searchCacheArchetypes(
                        itemRequiredMask,
                        cachedItemArchetypes,
                        itemArchetypesNumber
                    );

                    for (uint32_t c = 0; c < itemArchetypesNumber; ++c) {
                        arch::Archetype* arch = cachedItemArchetypes[c];
                        cm::transform* itemTransforms =
                            (ecs::components::transform*)arch->components
                                [arch::ComponentsIndices::TRANSFORM_COMPONENT];
                        cm::item* items =
                            (ecs::components::item*)arch->components
                                [arch::ComponentsIndices::ITEM_COMPONENT];
                        cm::material* itemMaterials =
                            (ecs::components::material*)arch->components
                                [arch::ComponentsIndices::MATERIAL_COMPONENT];
                        cm::mesh* itemMeshes =
                            (ecs::components::mesh*)arch->components
                                [arch::ComponentsIndices::MESH_COMPONENT];
                        cm::collider* itemColliders =
                            (ecs::components::collider*)arch->components
                                [arch::ComponentsIndices::COLLIDER_COMPONENT];

                        if (itemTransforms && itemMaterials && itemMeshes
                            && itemColliders && items) {
                            for (unsigned int a = 0; a < arch->entityCount;
                                 ++a) {
                                cm::item* itemComponent = &items[a];
                                if (!itemComponent->isActor) {
                                    vulkanRenderer->items.Push({});
                                    unsigned int meshID =
                                        itemMeshes[a].handle.id;
                                    unsigned int diffuseTexureID =
                                        itemMaterials[a].diffuseTextureID_.id;
                                    vulkanRenderer->items[itemCounter].meshID =
                                        meshID;
                                    vulkanRenderer->items[itemCounter]
                                        .diffuseTexureID = diffuseTexureID;
                                    cm::transform* itemTransformComponent =
                                        &itemTransforms[a];
                                    cm::collider* itemColliderComponent =
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
    arch::world.searchCacheArchetypes(
        crosshairRequiredMask,
        cachedCrosshairActorsArchetypes,
        crosshairActorsArchetypesNumber
    );

    for (uint32_t x = 0; x < crosshairActorsArchetypesNumber; ++x) {
        arch::Archetype* arch = cachedCrosshairActorsArchetypes[x];
        cm::transform* crosshairTransforms =
            (ecs::components::transform*)
                arch->components[arch::ComponentsIndices::TRANSFORM_COMPONENT];
        cm::mesh* crosshairMeshes =
            (ecs::components::mesh*)
                arch->components[arch::ComponentsIndices::MESH_COMPONENT];

        for (unsigned int i = 0; i < arch->entityCount; ++i) {
            vulkanRenderer->crosshairs.Push({});
            cm::transform* cursorTransform = &crosshairTransforms[i];
            unsigned int meshID = crosshairMeshes[i].handle.id;
            vulkanRenderer->crosshairs[i].meshID = meshID;
            vulkanRenderer->crosshairs[i].model =
                updateDataHudScreenUBO(cursorTransform);
        }
    }

    vulkanRenderer->actors.clear();
    levelChunkActorsArchetypesNumber = 0;
    arch::world.searchCacheArchetypes(
        levelChunkRequiredMask,
        cachedLevelChunkActorsArchetypes,
        levelChunkActorsArchetypesNumber
    );

    uint32_t levelChunkActorsCounter = 0;
    for (uint32_t x = 0; x < levelChunkActorsArchetypesNumber; ++x) {
        arch::Archetype* arch = cachedLevelChunkActorsArchetypes[x];
        cm::transform* levelChunkTransforms =
            (ecs::components::transform*)
                arch->components[arch::ComponentsIndices::TRANSFORM_COMPONENT];
        cm::mesh* levelChunkMeshes =
            (ecs::components::mesh*)
                arch->components[arch::ComponentsIndices::MESH_COMPONENT];
        cm::material* levelChunkMaterials =
            (ecs::components::material*)
                arch->components[arch::ComponentsIndices::MATERIAL_COMPONENT];
        cm::rotation* levelChunkRotations =
            (ecs::components::rotation*)
                arch->components[arch::ComponentsIndices::ROTATION_COMPONENT];
        ecs::tagComponents::levelChunkTagComponent* levelChunks =
            (ecs::tagComponents::levelChunkTagComponent*)arch
                ->components[arch::ComponentsIndices::LEVEL_CHUNK_TAG_COMPONENT];

        core::vector<mat4> jointMatrices;
        jointMatrices.Resize(MAX_JOINTS_NUMBER);
        for (unsigned int i = 0; i < MAX_JOINTS_NUMBER; ++i) {
            mat4 unitMatrix(1.0f);
            jointMatrices[i] = unitMatrix;
        }

        for (uint32_t n = 0; n < arch->entityCount; ++n) {
            vulkanRenderer->actors.Push({});
            cm::transform* transformComponent = &levelChunkTransforms[n];
            cm::material* materialComponent = &levelChunkMaterials[n];
            cm::rotation* rotationComponent = &levelChunkRotations[n];
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
    arch::world.searchCacheArchetypes(
        animationRequiredMask,
        cachedAnimationArchetypes,
        animationActorsArchetypesNumber
    );

    uint32_t animationActorsCounter = levelChunkActorsCounter;
    for (uint32_t x = 0; x < animationActorsArchetypesNumber; ++x) {
        arch::Archetype* arch = cachedAnimationActorsArchetypes[x];
        cm::transform* actorTransforms =
            (ecs::components::transform*)
                arch->components[arch::ComponentsIndices::TRANSFORM_COMPONENT];
        cm::mesh* actorMeshes =
            (ecs::components::mesh*)
                arch->components[arch::ComponentsIndices::MESH_COMPONENT];
        cm::material* actorMaterials =
            (ecs::components::material*)
                arch->components[arch::ComponentsIndices::MATERIAL_COMPONENT];
        cm::rotation* actorRotations =
            (ecs::components::rotation*)
                arch->components[arch::ComponentsIndices::ROTATION_COMPONENT];
        cm::animation* actorAnimations =
            (ecs::components::animation*)
                arch->components[arch::ComponentsIndices::ANIMATION_COMPONENT];

        for (uint32_t n = 0; n < arch->entityCount; ++n) {
            vulkanRenderer->actors.Push({});
            cm::transform* transformComponent = &actorTransforms[n];
            cm::material* materialComponent = &actorMaterials[n];
            [[maybe_unused]] cm::animation* animationComponent =
                &actorAnimations[n];
            cm::rotation* rotationComponent = &actorRotations[n];
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
    arch::world.searchCacheArchetypes(
        staticActorsRequiredMask,
        cachedStaticActorsArchetypes,
        staticActorsArchetypesNumber
    );

    uint32_t staticActorsCounter = animationActorsCounter;
    for (uint32_t x = 0; x < staticActorsArchetypesNumber; ++x) {
        arch::Archetype* arch = cachedStaticActorsArchetypes[x];
        cm::transform* staticActorTransforms =
            (ecs::components::transform*)
                arch->components[arch::ComponentsIndices::TRANSFORM_COMPONENT];
        cm::mesh* staticActorMeshes =
            (ecs::components::mesh*)
                arch->components[arch::ComponentsIndices::MESH_COMPONENT];
        cm::material* staticActorMaterials =
            (ecs::components::material*)
                arch->components[arch::ComponentsIndices::MATERIAL_COMPONENT];
        cm::rotation* staticActorRotations =
            (ecs::components::rotation*)
                arch->components[arch::ComponentsIndices::ROTATION_COMPONENT];

        core::vector<mat4> jointMatrices;
        jointMatrices.Resize(MAX_JOINTS_NUMBER);
        for (unsigned int i = 0; i < MAX_JOINTS_NUMBER; ++i) {
            mat4 unitMatrix(1.0f);
            jointMatrices[i] = unitMatrix;
        }

        for (uint32_t n = 0; n < arch->entityCount; ++n) {
            vulkanRenderer->actors.Push({});
            cm::transform* transformComponent = &staticActorTransforms[n];
            cm::material* materialComponent = &staticActorMaterials[n];
            cm::rotation* rotationComponent = &staticActorRotations[n];
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
    arch::world.searchCacheArchetypes(
        projectileRequiredMask,
        cachedProjectileActorsArchetypes,
        projectileActorsArchetypesNumber
    );

    uint32_t projectileActorsCounter = staticActorsCounter;
    for (uint32_t x = 0; x < projectileActorsArchetypesNumber; ++x) {
        arch::Archetype* arch = cachedProjectileActorsArchetypes[x];
        cm::transform* actorTransforms =
            (ecs::components::transform*)
                arch->components[arch::ComponentsIndices::TRANSFORM_COMPONENT];
        cm::mesh* actorMeshes =
            (ecs::components::mesh*)
                arch->components[arch::ComponentsIndices::MESH_COMPONENT];
        arch::ProjectileBundle* actorProjectileBundles =
            (arch::ProjectileBundle*)arch->components
                [arch::ComponentsIndices::PROJECTILE_BUNDLE_COMPONENT];
        cm::rotation* actorRotations =
            (ecs::components::rotation*)
                arch->components[arch::ComponentsIndices::ROTATION_COMPONENT];

        core::vector<mat4> jointMatrices;
        jointMatrices.Resize(MAX_JOINTS_NUMBER);
        for (unsigned int i = 0; i < MAX_JOINTS_NUMBER; ++i) {
            mat4 unitMatrix(1.0f);
            jointMatrices[i] = unitMatrix;
        }

        for (uint32_t n = 0; n < arch->entityCount; ++n) {
            vulkanRenderer->actors.Push({});
            cm::transform* transformComponent = &actorTransforms[n];
            cm::material* materialComponent =
                &actorProjectileBundles[n].material;
            cm::rotation* rotationComponent = &actorRotations[n];
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
    arch::world.searchCacheArchetypes(
        rotationItemRequiredMask,
        cachedItemActorsArchetypes,
        itemActorsArchetypesNumber
    );

    uint32_t itemActorsCounter = projectileActorsCounter;
    for (uint32_t x = 0; x < itemActorsArchetypesNumber; ++x) {
        arch::Archetype* arch = cachedItemActorsArchetypes[x];
        cm::transform* itemTransforms =
            (ecs::components::transform*)
                arch->components[arch::ComponentsIndices::TRANSFORM_COMPONENT];
        cm::mesh* itemMeshes =
            (ecs::components::mesh*)
                arch->components[arch::ComponentsIndices::MESH_COMPONENT];
        cm::material* itemMaterials =
            (cm::material*)
                arch->components[arch::ComponentsIndices::MATERIAL_COMPONENT];
        cm::rotation* itemRotations =
            (ecs::components::rotation*)
                arch->components[arch::ComponentsIndices::ROTATION_COMPONENT];
        cm::item* items =
            (ecs::components::item*)
                arch->components[arch::ComponentsIndices::ITEM_COMPONENT];

        core::vector<mat4> jointMatrices;
        jointMatrices.Resize(MAX_JOINTS_NUMBER);
        for (unsigned int i = 0; i < MAX_JOINTS_NUMBER; ++i) {
            mat4 unitMatrix(1.0f);
            jointMatrices[i] = unitMatrix;
        }

        for (uint32_t n = 0; n < arch->entityCount; ++n) {
            if (items[n].isActor) {
                vulkanRenderer->actors.Push({});
                cm::transform* transformComponent = &itemTransforms[n];
                cm::material* materialComponent = &itemMaterials[n];
                cm::rotation* rotationComponent = &itemRotations[n];
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
    arch::world.searchCacheArchetypes(
        playerRequiredMask,
        cachedPlayerArchetypes,
        playerArchetypesNumber
    );

    uint32_t playerEntityCount = 0;
    for (uint32_t x = 0; x < playerArchetypesNumber; ++x) {
        arch::Archetype* arch = cachedPlayerArchetypes[x];
        cm::transform* playerTransforms =
            (ecs::components::transform*)
                arch->components[arch::ComponentsIndices::TRANSFORM_COMPONENT];

        for (unsigned int n = 0; n < arch->entityCount; ++n) {
            vulkanRenderer->players.Push({});
            cm::transform* playerTransformComponent = &playerTransforms[n];
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

        vulkanRenderer->frames.Push({});
        vulkanRenderer->jointMatricesPerMesh.Push({});

        unsigned int vertexIndex = 0;
        unsigned int textureIndex = 0;
        unsigned int normalIndex = 0;
        unsigned int faceVerticesSize =
            wavefrontObjParser->getFaces().GetSize();
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

                vec4 jointIndices;
                vec4 weights;

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

                vulkanRenderer->aVertices_[m].Push(
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

void Engine::calculateMeshBounds(const vec4& animatedVertex) {
    if (animatedVertex[0] < vulkanRenderer->meshAxisLimitingValues.lowest_x) {
        vulkanRenderer->meshAxisLimitingValues.lowest_x = animatedVertex[0];
    } else if (
        animatedVertex[0] > vulkanRenderer->meshAxisLimitingValues.highest_x
    ) {
        vulkanRenderer->meshAxisLimitingValues.highest_x = animatedVertex[0];
    }

    if (animatedVertex[1] < vulkanRenderer->meshAxisLimitingValues.lowest_y) {
        vulkanRenderer->meshAxisLimitingValues.lowest_y = animatedVertex[1];
    } else if (
        animatedVertex[1] > vulkanRenderer->meshAxisLimitingValues.highest_y
    ) {
        vulkanRenderer->meshAxisLimitingValues.highest_y = animatedVertex[1];
    }

    if (animatedVertex[2] < vulkanRenderer->meshAxisLimitingValues.lowest_z) {
        vulkanRenderer->meshAxisLimitingValues.lowest_z = animatedVertex[2];
    } else if (
        animatedVertex[2] > vulkanRenderer->meshAxisLimitingValues.highest_z
    ) {
        vulkanRenderer->meshAxisLimitingValues.highest_z = animatedVertex[2];
    }
}

bool Engine::isModelCacheExists(const std::string& modelFilePath) {
    std::ofstream modelsCache(
        "../../../assets/cache/models/cache",
        std::ios::app
    );

    if (!modelsCache.is_open()) {
        std::cerr << "Error opening the models cache file" << std::endl;
        throw std::runtime_error("Failed to load mesh cache");
    }

    std::ifstream file("../../../assets/cache/models/cache");

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
        "../../../assets/cache/models/cache",
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
    core::vector<bool> animationFlags;
    for (unsigned int m = 0; m < pathsGLTF_.GetSize(); ++m) {
        Core::CJsonParser jsonParser;
        vulkanRenderer->aVertexesTemp_.emplace_back();
        vulkanRenderer->aIndices_.emplace_back();
        vulkanRenderer->frames.Push({});
        vulkanRenderer->jointMatricesPerMesh.Push({});
        animationFlags.Push({});
        vulkanRenderer->highest_gltf_Y.emplace_back();
        uint32_t nextIndexGLTF = wavefrontObjCounter + m;
        jsonParser.LoadGLTF(
            pathsGLTF_[m],
            vulkanRenderer->aVertexesTemp_[m],
            vulkanRenderer->aIndices_[nextIndexGLTF],
            vulkanRenderer->jointMatricesPerMesh[nextIndexGLTF],
            vulkanRenderer->frames[nextIndexGLTF],
            animationFlags[m],
            vulkanRenderer->highest_gltf_Y[nextIndexGLTF]
        );
    }

    for (unsigned int m = 0; m < pathsGLTF_.GetSize(); ++m) {
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

            vec4 joinIndices;
            vec4 weights;
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
            vulkanRenderer->aVertices_[nextIndexGLTF].Push(
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

            vec4 animatedVertex = vec4(vertex[0], vertex[1], vertex[2], 1.0);
            if (!animationFlags[m]
                && vulkanRenderer->jointMatricesPerMesh[nextIndexGLTF].GetSize()
                    > 0) {
                for (unsigned int frame = 0; frame
                     < vulkanRenderer->jointMatricesPerMesh[nextIndexGLTF][0]
                           .GetSize();
                     ++frame) {
                    mat4 skinMatrix =
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
                        vec4(vertex[0], vertex[1], vertex[2], 1.0) * skinMatrix;
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
            core::vector<Vertex> symbol_g_vertices;
            symbol_g_vertices.Push(
                {{-0.5f, 0.5f, 0.0f},
                 {0.0f, 1.0f, 0.0f},
                 {fontStep * j, fontStep * i + fontStep},
                 {0.0f, 0.0f, 0.0f, 0.0f},
                 {1.0f, 0.0f, 0.0f, 0.0f}}
            );
            symbol_g_vertices.Push(
                {{0.5f, 0.5f, 0.0f},
                 {1.0f, 1.0f, 0.0f},
                 {fontStep * j + fontStep, fontStep * i + fontStep},
                 {0.0f, 0.0f, 0.0f, 0.0f},
                 {1.0f, 0.0f, 0.0f, 0.0f}}
            );
            symbol_g_vertices.Push(
                {{-0.5f, -0.5f, 0.0f},
                 {0.0f, 0.0f, 0.0f},
                 {fontStep * j, fontStep * i},
                 {0.0f, 0.0f, 0.0f, 0.0f},
                 {1.0f, 0.0f, 0.0f, 0.0f}}
            );
            symbol_g_vertices.Push(
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

            vulkanRenderer->symbolGVerticesContainer.Push(symbol_g_vertices);
            vulkanRenderer->fontIndicesContainer.push_back(nextBufferIndex);
        }
    }
}

mat4 Engine::computeModelMatrix(
    ecs::components::transform* _transformComponent,
    ecs::components::rotation* rotation
) {
    mat4 rotationMatrix(1.0f);
    mat4 scalingMatrix(1.0f);
    mat4 translationMatrix(1.0f);

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

ecs::TextureHandle Engine::LoadTextureFromFile(const char* path_to_texture) {
    uint32_t textureID = textureVector.size();
    ecs::TextureHandle textureHandle;
    textureHandle.id = textureID;
    textureVector.push_back({.path_to_image = path_to_texture});
    textureHandlers.Push(textureHandle);

    return textureHandle;
}

ecs::TextureHandle Engine::LoadTextureFromAddress(
    unsigned int iWidth,
    unsigned int iHeight,
    unsigned int dat_length,
    unsigned char* u_iData
) {
    uint32_t textureID = textureVector.size();
    ecs::TextureHandle textureHandle;
    textureHandle.id = textureID;
    textureVector.push_back(
        {.iWidth_ = iWidth,
         .iHeight_ = iHeight,
         .dat_length_ = dat_length,
         .u_iData_ = u_iData}
    );
    textureHandlers.Push(textureHandle);

    return textureHandle;
}

ecs::components::MeshHandle Engine::LoadMeshFromFile_OBJ(
    const char* _pathToMesh
) {
    ecs::components::MeshHandle meshHandle;
    meshHandle.id = meshID;
    pathsArray_.push_back(_pathToMesh);
    meshHandlers.Push(meshHandle);
    ++meshID;

    return meshHandle;
}

ecs::components::MeshHandle Engine::LoadMeshFromFile_GLTF(
    const char* pathToMesh
) {
    ecs::components::MeshHandle meshHandle;
    meshHandle.id = meshID;
    pathsGLTF_.Push(pathToMesh);
    meshHandlers.Push(meshHandle);
    ++meshID;

    return meshHandle;
}

ecs::components::MeshHandle Engine::LoadMesh() {
    ecs::components::MeshHandle meshHandle;
    meshHandle.id = meshID;
    meshHandlers.Push(meshHandle);
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
} // namespace glvm::core
