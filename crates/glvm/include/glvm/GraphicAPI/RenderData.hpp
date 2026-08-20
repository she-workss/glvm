#pragma once

#include "glvm/Vector.hpp"
#include "glvm/VkStructs.hpp"

#include <vulkan/vulkan_core.h>

namespace glvm::core {
extern vector<VkDescriptorSet> descriptorSetsChunks;
extern glvm::core::vector<VkRenderPass> renderPasses;
extern glvm::core::vector<Descriptor> GPUDescriptors;
} // namespace glvm::core
