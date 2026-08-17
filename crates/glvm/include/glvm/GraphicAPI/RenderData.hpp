#ifndef RENDER_DATA
#define RENDER_DATA

#include "glvm/Vector.hpp"
#include "glvm/VkStructs.hpp"

#include <vulkan/vulkan_core.h>

namespace GLVM::core {
extern vector<VkDescriptorSet> descriptorSetsChunks;
extern GLVM::core::vector<VkRenderPass> renderPasses;
extern GLVM::core::vector<Descriptor> GPUDescriptors;
} // namespace GLVM::core

#endif
