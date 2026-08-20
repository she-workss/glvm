#pragma once

#include "glvm/GraphicAPI/RenderData.hpp"
#include "glvm/VertexMath.hpp"
#include "glvm/VkStructs.hpp"

#include <vulkan/vulkan_core.h>

namespace glvm::core {
class CVulkanRenderer;

struct DebugVertex {
    float x, y, z;
    float r, g, b;
};

class ImGuiOverlay {
public:
    explicit ImGuiOverlay(CVulkanRenderer& renderer);

    void init();
    void shutdown();
    void createSwapChainResources();
    void destroySwapChainResources();

    void newFrame();
    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

    bool wantsMouse() const;

    bool isEnabled() const {
        return initialized_;
    }

    bool showPanel = true;
    bool showActorBounds = false;
    bool showLightFrustums = false;
    bool showShadowMaps = false;
    bool showSpatialGrid = false;
    bool shadowsEnabled = true;
    int shadowMapMode = 0; // 0 = directional, 1 = spot.
    int shadowMapLight = 0;

private:
    CVulkanRenderer& renderer_;
    bool initialized_ = false;

    VkRenderPass renderPass_ = VK_NULL_HANDLE;
    std::vector<VkFramebuffer> framebuffers_;

    VkPipelineLayout lineLayout_ = VK_NULL_HANDLE;
    VkPipeline linePipeline_ = VK_NULL_HANDLE;

    VkBuffer vertexBuffer_ = VK_NULL_HANDLE;
    VkDeviceMemory vertexBufferMemory_ = VK_NULL_HANDLE;
    void* vertexBufferMapped_ = nullptr;
    uint32_t lineVertexCount_ = 0;

    void createRenderPass();
    void createLinePipeline();
    void createVertexBuffer();
    void buildPanel();
    void buildDebugVertices();
    void recordDebugDraws(VkCommandBuffer commandBuffer, uint32_t imageIndex);
    void recordImGuiDraws(VkCommandBuffer commandBuffer, uint32_t imageIndex);
};

} // namespace glvm::core
