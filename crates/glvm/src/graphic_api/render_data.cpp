#include "glvm/graphic_api/render_data.hpp"

namespace glvm::core {
std::vector<VkDescriptorSet> descriptorSetsChunks;
std::vector<VkRenderPass> renderPasses;
std::vector<Descriptor> GPUDescriptors;
} // namespace glvm::core
