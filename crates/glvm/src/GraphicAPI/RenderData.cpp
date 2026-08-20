#include "glvm/GraphicAPI/RenderData.hpp"

namespace glvm::core {
vector<VkDescriptorSet> descriptorSetsChunks;
glvm::core::vector<VkRenderPass> renderPasses;
glvm::core::vector<Descriptor> GPUDescriptors;
} // namespace glvm::core
