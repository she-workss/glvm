#pragma once

#include "glvm/GraphicAPI/RenderConfig.hpp"
#include "glvm/ToString.hpp"
#include "glvm/VkStructs.hpp"

#include <string>

namespace glvm::core::vkDebugUtils {
VkResult CreateDebugUtilsMessengerEXT(
    VkInstance instance,
    const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkDebugUtilsMessengerEXT* pDebugMessenger
);
void CreateBeginDebugUtilsLabelEXT(
    [[maybe_unused]] VkInstance instance,
    [[maybe_unused]] VkCommandBuffer commandBuffer,
    [[maybe_unused]] const VkDebugUtilsLabelEXT* labelInfo
);
void CreateEndDebugUtilsLabelEXT(
    [[maybe_unused]] VkInstance instance,
    [[maybe_unused]] VkCommandBuffer commandBuffer
);
void DestroyDebugUtilsMessengerEXT(
    VkInstance instance,
    VkDebugUtilsMessengerEXT debugMessenger,
    const VkAllocationCallbacks* pAllocator
);
VkResult SetDebugObjectName(
    VkDevice device,
    const VkDebugUtilsObjectNameInfoEXT* objectNameInfo
);
void setImageDebugObjectName(
    VkDevice device,
    VK_Image image,
    std::string imageName
);
void setPipelineDebugObjectName(
    VkDevice device,
    VkPipeline pipeline,
    std::string pipelineName
);
void setDescriptorSetObjectName(
    VkDevice device,
    VkDescriptorSet descriptorSet,
    std::string descriptorSetName,
    unsigned int index
);
void setDebugObjectNames(
    VkDevice device,
    const std::vector<VkBuffer>& vertexBufferContainer,
    const std::vector<VkBuffer>& indexBufferContainer,
    const glvm::core::vector<Descriptor>& GPUDescriptors,
    const std::vector<unsigned int>& fontIndicesContainer,
    const std::vector<VkBuffer>& fontVertexBufferContainer,
    const std::vector<VkBuffer>& fontIndexBufferContainer
);
}; // namespace glvm::core::vkDebugUtils
