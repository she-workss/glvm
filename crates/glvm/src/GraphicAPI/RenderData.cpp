#include "glvm/GraphicAPI/RenderData.hpp"

namespace GLVM::core {
vector<VkDescriptorSet> descriptorSetsChunks;
GLVM::core::vector<VkRenderPass> renderPasses;
GLVM::core::vector<Descriptor> GPUDescriptors;
} // namespace GLVM::core
