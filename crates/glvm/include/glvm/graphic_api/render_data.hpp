#pragma once

#include "glvm/vk_structs.hpp"

#include <vector>
#include <vulkan/vulkan_core.h>

namespace glvm::core {
extern std::vector<VkDescriptorSet> descriptorSetsChunks;
extern std::vector<VkRenderPass> renderPasses;
extern std::vector<Descriptor> GPUDescriptors;
} // namespace glvm::core
