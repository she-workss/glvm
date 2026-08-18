// This file is part of Game Loop Versatile Modules (GLVM)
// Copyright © 2024 Maksim Manokhin a.k.a. Yuriorkis_Scream. Contacts:
// <fellfrostqtw@gmail.com> Author: Maksim Manokhin a.k.a. Yuriorkis_Scream
// License: http://opensource.org/licenses/MIT

#include "glvm/GraphicAPI/Vulkan.hpp"

#include "glvm/ComponentManager.hpp"
#include "glvm/Components/ActorComponent.hpp"
#include "glvm/Components/AnimationMoveComponent.hpp"
#include "glvm/Components/ColliderComponent.hpp"
#include "glvm/Components/ControllerComponent.hpp"
#include "glvm/Components/CrosshairComponent.hpp"
#include "glvm/Components/HealthComponent.hpp"
#include "glvm/Components/InterfaceComponent.hpp"
#include "glvm/Components/InventoryComponent.hpp"
#include "glvm/Components/InventorySlotComponent.hpp"
#include "glvm/Components/ItemComponent.hpp"
#include "glvm/Components/MaterialComponent.hpp"
#include "glvm/Components/PointLightComponent.hpp"
#include "glvm/Components/TransformComponent.hpp"
#include "glvm/Components/VertexComponent.hpp"
#include "glvm/Components/ViewComponent.hpp"
#include "glvm/EntityManager.hpp"
#include "glvm/GraphicAPI/RenderConfig.hpp"
#include "glvm/GraphicAPI/RenderData.hpp"
#include "glvm/PGA.hpp"
#include "glvm/ShaderStructs.hpp"
#include "glvm/Texture.hpp"
#include "glvm/ThreadPool.hpp"
#include "glvm/UnixApi/WindowWaylandVulkan.hpp"
#include "glvm/Vector.hpp"
#include "glvm/VertexMath.hpp"
#include "glvm/VkStructs.hpp"
#include "glvm/WavefrontObjParser.hpp"

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <string>
#include <thread>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_wayland.h>

namespace GLVM::core {
CVulkanRenderer::CVulkanRenderer() {}

CVulkanRenderer::~CVulkanRenderer() {
    cleanup();
}

void CVulkanRenderer::draw() {
    mainRenderDrawFrame();
}

void CVulkanRenderer::SetViewMatrix(mat4 _viewMatrix) {
    viewMatrix = _viewMatrix; //
}

void CVulkanRenderer::SetProjectionMatrix(mat4 _projectionMatrix) {
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

        //			GPUDescriptors[descriptorBindingsConfig[readableTextureDescriptorBindingIndex].globalDescriptorOffset
        //+ i].GPUImage = new VK_Image;
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
}

void CVulkanRenderer::SetMeshData(
    std::vector<const char*> _pathsArray,
    core::vector<const char*> pathsGLTF
) {
    for (unsigned int i = 0; i < _pathsArray.size(); ++i) {
        pathsArray_.push_back(_pathsArray[i]);
    }

    for (unsigned int i = 0; i < pathsGLTF.GetSize(); ++i) {
        pathsGLTF_.Push(pathsGLTF[i]);
    }
}

void CVulkanRenderer::run() {
    VkConfigInitializer();
    descriptorSetBuilder();
    pipelineBuilder();
    renderPassesBuilder();
    for (int i = 0; i < 20; ++i) {
        //			std::cout << "descriptor offset: " <<
        // descriptorBindingsConfig[i].globalDescriptorOffset << std::endl;
    }

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
    Window = new GLVM::core::WindowXVulkan();
    createXlibSurfaceInfo.dpy = Window->GetDisplay();
    createXlibSurfaceInfo.window = Window->GetWindow();
    aspectRate = (float)Window->width / (float)Window->height;

    createXlibSurfaceInfo.sType =
        VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
    createXlibSurfaceInfo.pNext = nullptr;
    createXlibSurfaceInfo.flags = 0;
#endif

#ifdef VK_USE_PLATFORM_XCB_KHR
    Window = new GLVM::core::WindowXCBVulkan();
    createXcbSurfaceInfo.window = Window->GetWindow();
    createXcbSurfaceInfo.connection = Window->GetConnection();
    aspectRate = (float)Window->width / (float)Window->height;

    createXcbSurfaceInfo.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
    createXcbSurfaceInfo.pNext = nullptr;
    createXcbSurfaceInfo.flags = 0;
#endif

#ifdef VK_USE_PLATFORM_WIN32_KHR
    Window = new GLVM::core::WindowWinVulkan();
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
        jointMatricesPerMesh.Push({});
        frames.Push({});
        for (int i = 0; i < 64; ++i) {
            frames[frames.GetSize() - 1].Push(0.0f);
        }
        int maximumJoints = 64;
        core::vector<core::vector<mat4>> jointMatrices;
        for (int i = 0; i < maximumJoints; ++i) {
            core::vector<mat4> globalAllFrameNodeMatrix;
            int numberOfFrames = 64;
            for (int j = 0; j < numberOfFrames; ++j) {
                mat4 unitMatrix(1.0f);
                globalAllFrameNodeMatrix.Push(unitMatrix);
            }

            jointMatrices.Push(globalAllFrameNodeMatrix);
        }
        jointMatricesPerMesh[jointMatricesPerMesh.GetSize() - 1] =
            jointMatrices;

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
    vkDebugUtils::setDebugObjectNames(
        device,
        vertexBufferContainer,
        indexBufferContainer,
        GPUDescriptors,
        fontIndicesContainer,
        fontVertexBufferContainer,
        fontIndexBufferContainer
    );
    // createCommandBuffers(mainRenderCommandPool,
    // directionalLightCommandBuffers);
    // createCommandBuffers(mainRenderCommandPool, spotLightCommandBuffers);
    // createCommandBuffers(mainRenderCommandPool, pointLightCommandBuffers);
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
    // createSyncObjects(directionalLightShadowMapImageAvailableSemaphores,
    // 				  directionalLightShadowMapRenderFinishedSemaphores,
    // 				  directionalLightShadowMapInFlightFences);
    // createSyncObjects(spotLightShadowMapImageAvailableSemaphores,
    // 				  spotLightShadowMapRenderFinishedSemaphores,
    // 				  spotLightShadowMapInFlightFences);
    // createSyncObjects(pointLightShadowMapImageAvailableSemaphores,
    // 				  pointLightShadowMapRenderFinishedSemaphores,
    // 				  pointLightShadowMapInFlightFences);
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
    for (unsigned int m = 0; m < pathsGLTF_.GetSize(); ++m) {
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
    for (unsigned int i = 0; i < symbolGVerticesContainer.GetSize(); ++i) {
        const unsigned int nextBufferIndex = fontIndicesContainer[i];
        core::vector<Vertex> symbol_g_vertices = symbolGVerticesContainer[i];

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

    for (unsigned int i = 0, j = 0; i < GPUDescriptors.GetSize(); ++j) {
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
        //            vkDestroySemaphore(device, renderFinishedSemaphores[i],
        //            nullptr);
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
        vkDebugUtils::DestroyDebugUtilsMessengerEXT(
            instance,
            debugMessenger,
            nullptr
        );
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

    if (vkDebugUtils::CreateDebugUtilsMessengerEXT(
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
            // std::cout << prop.deviceType << std::endl;
            // std::cout << prop.deviceName << std::endl;
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
        indices.graphicsFamily.value(),
        indices.presentFamily.value()
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

    vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
    vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
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
        indices.graphicsFamily.value(),
        indices.presentFamily.value()
    };

    if (indices.graphicsFamily != indices.presentFamily) {
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
        //			std::cout << "PIPELINE NUMBER: " << j << std::endl;
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
        // std::cout << "NEXT DS" << std::endl;
        // std::cout << "binding count: " <<
        // descriptorSet.actualLinkedDescriptorBindingsNumber << std::endl;
        for (u32 j = 0; j < descriptorSet.actualLinkedDescriptorBindingsNumber;
             ++j) {
            u32 currentDescriptorBindingID =
                descriptorSet.descriptorsBindingsIDs[j];
            //			u32 currentDescriptorBindingID = j;
            //				std::cout << "DS ID: " << currentDescriptorBindingID
            //<< std::endl;
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
        //			std::cout << "NUMBER OF BINDINGS: " <<
        // static_cast<uint32_t>(bindings.size()) << std::endl;
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
        //			std::cout << "shader: " << pipeline.vertShader << std::endl;
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
        // meshes are CW-wound; main pass keeps outer faces (cull "front"),
        // shadow pass keeps far-side faces (cull "back") so objects do not
        // self-shadow
        if (isShadowMapPipeline) {
            rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
        } else if (
            graphicsPipelineCounter == SpecificPipeline::MAIN_RENDER_PIPELINE
        ) {
            rasterizer.cullMode = VK_CULL_MODE_FRONT_BIT;
        } else {
            // 2D/UI pipelines (HUD, crosshair, fonts, SDF) draw screen-space
            // quads; culling them makes the sprites disappear
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

        /*
          Need to access inside pipeline and take ID for specific descriptor
          set, then with that ID we got descriptor set and take it's layout
        */
        unsigned int descriptorLayoutsNumber =
            pipeline.actualLinkedDescriptorSetsNumber;
        core::vector<VkDescriptorSetLayout> descriptorSetLayouts;
        for (unsigned i = 0; i < descriptorLayoutsNumber; ++i) {
            //				std::cout << "ds inside pipeline: " <<
            // pipeline.linkedDescriptorSetIDs[i] << std::endl;
            descriptorSetLayouts.Push(
                descriptorSetsConfig[pipeline.linkedDescriptorSetIDs[i]].setLayout
            );
        }

        VkPipelineLayoutCreateInfo pipelineLayoutInfo {};
        pipelineLayoutInfo.sType =
            VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = descriptorLayoutsNumber;
        pipelineLayoutInfo.pSetLayouts =
            descriptorSetLayouts.GetVectorContainer();

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
    /// Main renderer frame buffers initialization
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

    /// Directional lights shadow map renderer frame buffers initialization
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

    /// Spot lights shadow map renderer frame buffers initialization
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

    /// Point lights shadow map renderer frame buffers initialization
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
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

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
        vkDebugUtils::setImageDebugObjectName(
            device,
            depthImage,
            "directional light"
        );
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
        vkDebugUtils::setImageDebugObjectName(device, depthImage, "spot light");
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
            //				barrier.newLayout =
            // VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
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

        vkDebugUtils::setImageDebugObjectName(
            device,
            depthImage,
            "point light laryer"
        );
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

        vkDebugUtils::setImageDebugObjectName(
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
    core::vector<Vertex>& _vertices
) {
    VkDeviceSize bufferSize = sizeof(_vertices[0]) * _vertices.GetSize();
    if (_vertices.GetSize() == 0) {
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
    if (_vertices.GetSize() > 0) {
        memcpy(data, _vertices.GetVectorContainer(), (size_t)bufferSize);
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

// void CVulkanRenderer::createMemoryArenaBuffers(VkBuffer buffer,
// VkDeviceMemory deviceMemory, VkDeviceSize, 	) {

// }

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
                u32 memory =
                    descriptorBindingsConfig[descriptorBindingIndex].uboChunkSize
                    * descriptorSetsConfig[descriptorSetConfigCounter]
                          .hostDescriptorNumber;
                // std::cout << "ds binding index: " << descriptorBindingIndex
                // << std::endl; std::cout << "host ds number: " <<
                // descriptorSetsConfig[descriptorBindingIndex].hostDescriptorNumber
                // << std::endl; std::cout << "chunk size: " <<
                // descriptorBindingsConfig[descriptorBindingIndex].uboChunkSize
                // << std::endl; std::cout << "MEMORY: " << memory << std::endl;
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
    core::vector<VkDescriptorSet>& descriptorSets,
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

    //		descriptorSets.Resize(descriptorSetsNumber);
    if (vkAllocateDescriptorSets(
            device,
            &allocInfo,
            descriptorSets.GetVectorContainer() + descriptorOffset
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
    [[maybe_unused]] core::vector<VkDescriptorSet>& uboDescriptorSets,
    const unsigned int offset
) {
    for (size_t i = 0; i < uboDescriptorsNumber; ++i) {
        VkDescriptorBufferInfo modelMatrixBufferInfo =
            createDescriptorBufferInfo(ubo, uboStructSize, i);
        std::array<VkWriteDescriptorSet, 1> descriptorWrites {};

        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet =
            *(descriptorSetsChunks.GetVectorContainer() + offset + i);
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
    core::vector<u32> shaderBindings;
    core::vector<u32> descriptorNumberPerBinding;
    core::vector<u32> bindingsIDs;
    for (size_t j = 0; j < linkedDescriptorSetBindingsNumber; ++j) {
        shaderBindings.Push(
            descriptorBindingsConfig[currentDescriptorSet1
                                         .descriptorsBindingsIDs[j]]
                .binding
        );
        bindingsIDs.Push(currentDescriptorSet1.descriptorsBindingsIDs[j]);
    }

    for (size_t i = 0; i < currentDescriptorSet1.hostDescriptorNumber; ++i) {
        core::vector<VkWriteDescriptorSet> descriptorWrites;
        descriptorWrites.Resize(shaderBindings.GetSize());
        core::vector<VkDescriptorBufferInfo> descriptorBufferInfos;
        core::vector<core::vector<VkDescriptorImageInfo>> descriptorImageInfos;
        for (size_t j = 0; j < shaderBindings.GetSize(); ++j) {
            if (descriptorBindingsConfig[bindingsIDs[j]].vkType
                == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) {
                descriptorBufferInfos.Push({});

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
                descriptorWrites[j].pBufferInfo =
                    descriptorBufferInfos.GetVectorContainer();
            } else if (
                descriptorBindingsConfig[bindingsIDs[j]].vkType
                == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
            ) {
                descriptorImageInfos.Push({});

                for (size_t m = 0; m < descriptorBindingsConfig[bindingsIDs[j]]
                                           .shaderDescriptorsNumber;
                     ++m) {
                    u32 imageViewIndex =
                        GPUDescriptors
                            [descriptorBindingsConfig[bindingsIDs[j]]
                                 .globalDescriptorOffset
                             + m]
                                .GPUImage->views.size()
                        - 1;

                    descriptorImageInfos[descriptorImageInfos.GetSize() - 1]
                        .Push({});
                    descriptorImageInfos[descriptorImageInfos.GetSize() - 1][m] =
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
                    descriptorImageInfos[descriptorImageInfos.GetSize() - 1]
                        .GetVectorContainer();
            }
            descriptorWrites[j].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[j].dstSet =
                *(descriptorSetsChunks.GetVectorContainer()
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
            static_cast<uint32_t>(descriptorWrites.GetSize()),
            descriptorWrites.GetVectorContainer(),
            0,
            nullptr
        );
    }
}

void CVulkanRenderer::updateDescriptorSetsCombinedImageSampler(
    const DescriptorSet& descriptorSet
) {
    core::vector<u32> bindingsIDs;
    for (size_t j = 0; j < descriptorSet.actualLinkedDescriptorBindingsNumber;
         ++j) {
        bindingsIDs.Push(descriptorSet.descriptorsBindingsIDs[j]);
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
        core::vector<VkWriteDescriptorSet> descriptorWrites {};

        for (unsigned int j = 0; j < bindingsIDs.GetSize(); ++j) {
            descriptorWrites.Push({});
            const unsigned int lastElement = descriptorWrites.GetSize() - 1;
            descriptorWrites[lastElement].sType =
                VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[lastElement].dstSet =
                *(descriptorSetsChunks.GetVectorContainer()
                  + descriptorSet.descriptorSetOffset + i);
            descriptorWrites[lastElement].dstBinding =
                descriptorBindingsConfig[bindingsIDs[j]].binding;
            ;
            descriptorWrites[lastElement].dstArrayElement = 0;
            descriptorWrites[lastElement].descriptorType =
                VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorWrites[lastElement].descriptorCount = 1;
            descriptorWrites[lastElement].pImageInfo = &imageInfo;
        }
        vkUpdateDescriptorSets(
            device,
            static_cast<uint32_t>(descriptorWrites.GetSize()),
            descriptorWrites.GetVectorContainer(),
            0,
            nullptr
        );
    }
}

void CVulkanRenderer::createDescriptorImageInfo(
    const unsigned int descriptorNumber,
    VkImageLayout imageLayout,
    core::vector<VK_Image>& textureImages,
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

    i32 result = vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory);
    if (result != VK_SUCCESS) {
        //			std::cout << "result" << result << std::endl;
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
        //            if ((typeFilter & (1 << i)) &&
        //            (memProperties.memoryTypes[i].propertyFlags & properties)
        //            == properties && memProperties.memoryTypes[i].heapIndex ==
        //            0) {
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

    // if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
    //     throw std::runtime_error("failed to begin recording command buffer!");
    // }

    //		CreateEndDebugUtilsLabelEXT(instance, commandBuffer);

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

    for (unsigned int i = 0; i < healthBars.GetSize(); ++i) {
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
            &(*(descriptorSetsChunks.GetVectorContainer()
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

    // if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
    //     throw std::runtime_error("failed to record command buffer!");
    // }
}

void CVulkanRenderer::uiRecordCommandBuffer(
    VkCommandBuffer& commandBuffer,
    uint32_t imageIndex
) {
    VkCommandBufferBeginInfo beginInfo {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    // if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
    //     throw std::runtime_error("failed to begin recording command buffer!");
    // }

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

    for (unsigned int i = 0; i < inventories.GetSize(); ++i) {
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
                    &(*(descriptorSetsChunks.GetVectorContainer()
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
                    &(*(descriptorSetsChunks.GetVectorContainer()
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

    // if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
    //     throw std::runtime_error("failed to record command buffer!");
    // }
}

void CVulkanRenderer::uiIconsRecordCommandBuffer(
    VkCommandBuffer& commandBuffer,
    uint32_t imageIndex
) {
    VkCommandBufferBeginInfo beginInfo {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    // if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
    //     throw std::runtime_error("failed to begin recording command buffer!");
    // }

    //		CreateEndDebugUtilsLabelEXT(instance, commandBuffer);

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

    for (unsigned int i = 0; i < items.GetSize(); ++i) {
        RenderItem item = items[i];
        unsigned int uiVertexId = item.meshID;
        unsigned int diffuseTexureID = item.diffuseTexureID;
        unsigned int uboIndex = currentFrame * items.GetSize() + i;

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
            &(*(descriptorSetsChunks.GetVectorContainer()
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
            &(*(descriptorSetsChunks.GetVectorContainer()
                + currentDescriptorSet1.descriptorSetOffset
                + MAX_FRAMES_IN_FLIGHT * diffuseTexureID + currentFrame)),
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

    // if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
    //     throw std::runtime_error("failed to record command buffer!");
    // }
}

void CVulkanRenderer::hudScreenRecordCommandBuffer(
    VkCommandBuffer& commandBuffer,
    uint32_t imageIndex
) {
    VkCommandBufferBeginInfo beginInfo {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    // if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
    //     throw std::runtime_error("failed to begin recording command buffer!");
    // }

    //		CreateEndDebugUtilsLabelEXT(instance, commandBuffer);

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

    for (unsigned int i = 0; i < crosshairs.GetSize(); ++i) {
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
            &(*(descriptorSetsChunks.GetVectorContainer()
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

    // if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
    //     throw std::runtime_error("failed to begin recording command buffer!");
    // }

    //		CreateEndDebugUtilsLabelEXT(instance, commandBuffer);

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

    for (unsigned int i = 0; i < crosshairs.GetSize(); ++i) {
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
            &(*(descriptorSetsChunks.GetVectorContainer()
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

        //			unsigned int indicesContainerSize =
        // aIndices_[uiVertexId].size();

        //			vkCmdDrawIndexed(commandBuffer,
        // static_cast<uint32_t>(indicesContainerSize), 1, 0, 0, 0);
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

    // if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
    //     throw std::runtime_error("failed to begin recording command buffer!");
    // }

    //		CreateEndDebugUtilsLabelEXT(instance, commandBuffer);

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

    //		RenderPlayer player = {};
    for (unsigned int playerCounter = 0; playerCounter < players.GetSize();
         ++playerCounter) {
        player = players[playerCounter];
    }
    unsigned int currentActorMemoryOffset = 0;
    for (unsigned int i = 0; i < fonts.GetSize(); ++i) {
        RenderFont font = fonts[i];
        vec3 playerTragetDirection = font.position - player.position;
        float dotProduct = Dot(playerTragetDirection, player.forward);
        if (dotProduct <= 0) {
            continue;
        }

        for (unsigned int j = 0; j < font.font_string.GetSize(); ++j) {
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
            vec3 result;
            vec4 pos =
                vec4(font.position[0], font.position[1], font.position[2], 1.0f);

            vec4 clipSpacePosition = pos * viewMatrix * projectionMatrix;
            vec3 ndcPosition = vec3(
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
                &(*(descriptorSetsChunks.GetVectorContainer()
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
                &(*(descriptorSetsChunks.GetVectorContainer()
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
        currentActorMemoryOffset +=
            currentFrame * fontUboDescriptorNumber + font.font_string.GetSize();
    }
    //		}

    vkCmdEndRenderPass(commandBuffer);

    // if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
    //     throw std::runtime_error("failed to record command buffer!");
    // }
}

void CVulkanRenderer::recordCommandBuffer(
    VkCommandBuffer& commandBuffer,
    uint32_t imageIndex
) {
    // VkCommandBufferBeginInfo beginInfo{};
    // beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    // if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
    //     throw std::runtime_error("failed to begin recording command buffer!");
    // }

    namespace cm = GLVM::ecs::components;
    //		vkDebugUtils::CreateEndDebugUtilsLabelEXT(instance, commandBuffer);

    VkRenderPassBeginInfo renderPassInfo {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass =
        renderPasses[SpecificPipeline::MAIN_RENDER_PIPELINE];
    renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent.height = swapChainExtent.height;
    renderPassInfo.renderArea.extent.width = swapChainExtent.width;

    for (unsigned int player = 0; player < players.GetSize(); ++player) {
        updateViewPositionUniformBuffer(currentFrame, player);
    }

    std::array<VkClearValue, 2> clearValues {};
    if (players.GetSize() == 0) {
        clearValues[0].color = {
            {0.7f, 0.2f, 0.2f, 1.0f}
        }; ///< Player death screen
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

    for (unsigned int i = 0; i < actors.GetSize(); ++i) {
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
            &(*(descriptorSetsChunks.GetVectorContainer()
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
            &(*(descriptorSetsChunks.GetVectorContainer()
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
            &(*(descriptorSetsChunks.GetVectorContainer()
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
            &(*(descriptorSetsChunks.GetVectorContainer()
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
    //		}

    vkCmdEndRenderPass(commandBuffer);

    // if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
    //     throw std::runtime_error("failed to record command buffer!");
    // }
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

    //		projectionMatrixCubeShadowMap[1][1] *= -1;

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

    /// Start of animation logic
    for (unsigned int j = 0; j < MAX_JOINTS_NUMBER; ++j) {
        modelMatrixUBO.jointMatrices[j] = actors[actor].jointMatrices[j];
    }
    /// End of animation logic

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
    directionalLightNumber = directionalLights.GetSize();
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

    pointLightNumber = pointLights.GetSize();
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
    spotLightNumber = spotLights.GetSize();
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
                //				randomTileIndex = 20;
                indirectTexture[i][j] = randomTileIndex;
                // if( print )
                // 	std::cout << "element: " << i * 4 + j << " value: " <<
                // indirectTexture[i][j] << std::endl;

                // std::cout << "index: " << i * INDIRECT_TEXTURE_WIDTH * 4 + j
                // * 4 + 3 << std::endl; lightDataUBO.indirectTexture[i *
                // INDIRECT_TEXTURE_WIDTH * 4 + j * 4 + 3] = randomTileIndex;
                //				std::cout << "element: " << i *
                // INDIRECT_TEXTURE_WIDTH + j << " equal: " <<
                // lightDataUBO.indirectTexture[i * INDIRECT_TEXTURE_WIDTH + j]
                //<< std::endl;
            }
        }
    }
    print = false;
    // if( print == true )
    // 	print = false;

    lightDataUBO.tilesetTilesCount = vec2(TILESET_ROW, TILESET_COLUMN);
    lightDataUBO.tilesRaw = 8;
    lightDataUBO.tilesColumn = 8;
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
    namespace cm = GLVM::ecs::components;
    vkWaitForFences(
        device,
        1,
        &inFlightFences[currentFrame],
        VK_TRUE,
        UINT64_MAX
    );

    uint32_t imageIndex;
    /* vkAcquireNextImageKHR give index of image that WILL BE SOON available for
       rendering and signal imageAvailablesemaphore when its so. GraphicsQueue
       waint for this semaphore bacause we pass it in submitInfo.
     */
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
        /*VkCommandBufferResetFlagBits*/ 0
    );
    // directionalLightRecordCoomandBuffer(directionalLightSecondaryCommandBuffers,
    // currentFrame);
    // spotLightRecordCommandBuffer(spotLightSecondaryCommandBuffers,
    // currentFrame);
    // pointLightRecordCommandBuffer(pointLightSecondaryCommandBuffers,
    // currentFrame);

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
         directionalLightCounter < directionalLights.GetSize();
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
    for (uint32_t spotLightCounter = 0; spotLightCounter < spotLights.GetSize();
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
    for (uint32_t pointLightCounter = 0;
         pointLightCounter < pointLights.GetSize();
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

    hudScreenRecordCommandBuffer(
        mainRenderCommandBuffers[currentFrame],
        imageIndex
    );
    //		sdfRecordCommandBuffer(mainRenderCommandBuffers[currentFrame],
    // imageIndex);

    VkSubmitInfo submitInfo {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    /// GraphicsQueue wait for swapchain image when its become available.
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
    //		currentFrame = 0;
}

void CVulkanRenderer::directionalLightShadowMapDrawFrame() {
    namespace cm = GLVM::ecs::components;
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
        /*VkCommandBufferResetFlagBits*/ 0
    );
    //        directionalLightRecordCoomandBuffer(directionalLightCommandBuffers[directionalLightCurrentFrame],
    //        imageIndex);

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
    namespace cm = GLVM::ecs::components;
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
        /*VkCommandBufferResetFlagBits*/ 0
    );
    //        spotLightRecordCommandBuffer(spotLightCommandBuffers[spotLightCurrentFrame],
    //        imageIndex);

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
    namespace cm = GLVM::ecs::components;
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
        /*VkCommandBufferResetFlagBits*/ 0
    );
    //        pointLightRecordCommandBuffer(pointLightCommandBuffers[pointLightCurrentFrame],
    //        imageIndex);

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
         directionalLightCounter < directionalLights.GetSize();
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

        // VkClearValue shadowMapClearValues[1];
        // shadowMapClearValues[0].depthStencil.depth = 1.0f;
        // shadowMapClearValues[0].depthStencil.stencil = 0;

        // VkRenderPassBeginInfo shadowMapRenderPassInfo{};
        // shadowMapRenderPassInfo.sType =
        // VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        // shadowMapRenderPassInfo.pNext = NULL;
        // shadowMapRenderPassInfo.renderPass =
        // renderPasses[SpecificPipeline::DIRECTIONAL_LIGHT_PIPELINE];
        // shadowMapRenderPassInfo.framebuffer =
        // directionalLightShadowMapFrameBuffers[directionalLightCounter];
        // shadowMapRenderPassInfo.renderArea.offset.x = 0;
        // shadowMapRenderPassInfo.renderArea.offset.y = 0;
        // shadowMapRenderPassInfo.renderArea.extent.width =
        // swapChainExtent.width;
        // shadowMapRenderPassInfo.renderArea.extent.height =
        // swapChainExtent.height; shadowMapRenderPassInfo.clearValueCount = 1;
        // shadowMapRenderPassInfo.pClearValues = shadowMapClearValues;

        // vkCmdBeginRenderPass(commandBuffer, &shadowMapRenderPassInfo,
        // VK_SUBPASS_CONTENTS_INLINE);

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

        uint32_t actorsNumber = actors.GetSize();
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
                &(*(descriptorSetsChunks.GetVectorContainer()
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
        //			vkCmdEndRenderPass(commandBuffer);
    }
}

void CVulkanRenderer::spotLightRecordCommandBuffer(
    std::vector<VkCommandBuffer>& commandBuffers,
    [[maybe_unused]] uint32_t currentFrame
) {
    for (uint32_t spotLightCounter = 0; spotLightCounter < spotLights.GetSize();
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

        // VkClearValue spotLightShadowMapClearValues[1];
        // spotLightShadowMapClearValues[0].depthStencil.depth = 1.0f;
        // spotLightShadowMapClearValues[0].depthStencil.stencil = 0;

        // VkRenderPassBeginInfo spotLightShadowMapRenderPassInfo{};
        // spotLightShadowMapRenderPassInfo.sType =
        // VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        // spotLightShadowMapRenderPassInfo.pNext = NULL;
        // spotLightShadowMapRenderPassInfo.renderPass =
        // renderPasses[SpecificPipeline::SPOT_LIGHT_PIPELINE];
        // spotLightShadowMapRenderPassInfo.framebuffer =
        // spotLightShadowMapFrameBuffers[spotLightCounter];
        // spotLightShadowMapRenderPassInfo.renderArea.offset.x = 0;
        // spotLightShadowMapRenderPassInfo.renderArea.offset.y = 0;
        // spotLightShadowMapRenderPassInfo.renderArea.extent.width =
        // swapChainExtent.width;
        // spotLightShadowMapRenderPassInfo.renderArea.extent.height =
        // swapChainExtent.height;
        // spotLightShadowMapRenderPassInfo.clearValueCount = 1;
        // spotLightShadowMapRenderPassInfo.pClearValues =
        // spotLightShadowMapClearValues;

        // vkCmdBeginRenderPass(commandBuffer,
        // &spotLightShadowMapRenderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

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
        uint32_t actorsNumber = actors.GetSize();
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
                &(*(descriptorSetsChunks.GetVectorContainer()
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
        //			vkCmdEndRenderPass(commandBuffer);
    }
}

void CVulkanRenderer::pointLightRecordCommandBuffer(
    std::vector<VkCommandBuffer>& commandBuffers,
    [[maybe_unused]] uint32_t currentFrame
) {
    // 		if ( entityManager->isEntitiesCollectionChanged &&
    // componentManager->isComponentsCollectionChanged ) {

    // 			core::vector<unsigned int> linkedEntities;
    // 			for ( unsigned int i = 0; i < linkedEntitiesTemp.GetSize(); ++i
    // ) { 				unsigned int entity = linkedEntitiesTemp[i];
    // for ( unsigned int j = 0; j < pointLightEntities.GetSize(); ++j ) {
    // if ( entity == pointLightEntities[j] ) { 						break;
    // } else if ( entity != pointLightEntities[j] && j ==
    // pointLightEntities.GetSize() - 1 ) {
    // linkedEntities.Push(entity);
    // 					}
    // 				}
    // 			}
    // //			std::cout << "number of actors: " <<
    // linkedEntities.GetSize() << std::endl;
    // 			entitiesCollectionLinked__Trn_Mat_Mes_Act.clear();
    // 			for ( unsigned int i = 0; i < linkedEntities.GetSize(); ++i )
    // 				entitiesCollectionLinked__Trn_Mat_Mes_Act.Push(linkedEntities[i]);

    // 			entitiesCollectionLinked__Trn_PoL_Mes_Act.clear();
    // 			for ( unsigned int i = 0; i < pointLightEntities.GetSize(); ++i
    // )
    // entitiesCollectionLinked__Trn_PoL_Mes_Act.Push(pointLightEntities[i]);

    // 			entityManager->isEntitiesCollectionChanged = false;
    // 			componentManager->isComponentsCollectionChanged = false;
    // 		}

    // VkDebugUtilsLabelEXT label;
    // label.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
    // label.color[0] = 0.1;
    // label.color[0] = 0.7;
    // label.color[0] = 0.2;
    // label.color[0] = 1.0;
    // label.pLabelName = "pointLightShadowMap";
    // label.pNext = NULL;

    // vkDebugUtils::CreateBeginDebugUtilsLabelEXT(instance, commandBuffers[0],
    // &label);
    for (uint32_t pointLightCounter = 0;
         pointLightCounter < pointLights.GetSize();
         ++pointLightCounter) {
        uint32_t maxCubeMapLayers = 6;
        for (uint32_t cubeMapLayerCounter = 0;
             cubeMapLayerCounter < maxCubeMapLayers;
             ++cubeMapLayerCounter) { ///< 6 is a number of cube map layers.
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

            // VkClearValue pointLightShadowMapClearValues[2];
            // pointLightShadowMapClearValues[0].depthStencil.depth = 1.0f;
            // pointLightShadowMapClearValues[0].depthStencil.stencil = 0;
            // pointLightShadowMapClearValues[1].color = {{0.5f, 0.5f,
            // 0.5f, 1.0f}};

            // VkRenderPassBeginInfo pointLightShadowMapRenderPassInfo{};
            // pointLightShadowMapRenderPassInfo.sType =
            // VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            // pointLightShadowMapRenderPassInfo.pNext = NULL;
            // pointLightShadowMapRenderPassInfo.renderPass =
            // renderPasses[SpecificPipeline::POINT_LIGHT_PIPELINE];
            // pointLightShadowMapRenderPassInfo.framebuffer =
            // pointLightShadowMapFrameBuffers[pointLightCounter][cubeMapLayerCounter];
            // pointLightShadowMapRenderPassInfo.renderArea.offset.x = 0;
            // pointLightShadowMapRenderPassInfo.renderArea.offset.y = 0;
            // pointLightShadowMapRenderPassInfo.renderArea.extent.width =
            // SHADOW_MAP_SIZE;
            // pointLightShadowMapRenderPassInfo.renderArea.extent.height =
            // SHADOW_MAP_SIZE; pointLightShadowMapRenderPassInfo.clearValueCount
            // = 2; pointLightShadowMapRenderPassInfo.pClearValues =
            // pointLightShadowMapClearValues;

            // vkCmdBeginRenderPass(commandBuffer,
            // &pointLightShadowMapRenderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

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

            //				unsigned int pointLightEntity =
            // entitiesCollectionLinked__Trn_PoL_Mes_Act[pointLightCounter];

            uint32_t actorsNumber = actors.GetSize();
            for (unsigned int actorCounter = 0; actorCounter < actorsNumber;
                 ++actorCounter) {
                //					unsigned int meshOwnerEntity =
                // entitiesCollectionLinked__Trn_Mat_Mes_Act[actorCounter];
                RenderActor actor = actors[actorCounter];
                unsigned int meshID = actor.meshID;

                unsigned int uboIndex = pointLightNumber * actorsNumber
                        * maxCubeMapLayers * pointLightCurrentFrame
                    + ///< Choose frame (first 168 or second 168)
                    actorsNumber * maxCubeMapLayers * pointLightCounter
                    + ///< Choose point light (i)
                    maxCubeMapLayers * actorCounter
                    + cubeMapLayerCounter; ///< Choose actor (m) and layer (j)

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
                    &(*(descriptorSetsChunks.GetVectorContainer()
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
            //				vkCmdEndRenderPass(commandBuffer);
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
            //			if (availablePresentMode == VK_PRESENT_MODE_FIFO_KHR) {
            //				std::cout << "present mode found!" << std::endl;
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
            indices.graphicsFamily = i;
        }

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(
            device,
            i,
            surface,
            &presentSupport
        );

        if (presentSupport) {
            indices.presentFamily = i;
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
    //		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
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
    // (void) messageSeverity;
    // (void) messageType;
    // (void) pUserData;
    // if ( pCallbackData->messageIdNumber == 941228658 ) {
    // 	[[maybe_unused]] int i = 0;
    // }

    // std::cout << "Error code: " << pCallbackData->messageIdNumber <<
    // std::endl; std::cout << "Message name:: " <<
    // pCallbackData->pMessageIdName << std::endl;
    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

    return VK_FALSE;
}
} // namespace GLVM::core
