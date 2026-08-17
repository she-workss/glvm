#include "glvm/GraphicAPI/RenderConfig.hpp"
#include "glvm/GraphicAPI/RenderData.hpp"
#include "glvm/VkStructs.hpp"
#include "glvm/typenames.hpp"

namespace GLVM::core {
void descriptorSetBuilder() {
    static unsigned int DS_globalBindingsCounter =
        0; ///< Counts ds bindings indexes inside ds
    static unsigned int DS_hostNumber = 0; ///< Counts host data ds
    static unsigned int globalDescriptorsOffset =
        0; ///< Counts offsets data descriptors

    for (unsigned int dsCounter = 0;
         dsCounter < DescriptorSetDataLink::DESCRIPTOR_CHUNKS_NUMBER;
         ++dsCounter) {
        /// Offset for indexing inside descriptorSetsChunks
        descriptorSetsConfig[dsCounter].descriptorSetOffset = DS_hostNumber;
        DS_hostNumber += descriptorSetsConfig[dsCounter].hostDescriptorNumber;

        for (unsigned int DS_localBindingsCounter = 0; DS_localBindingsCounter
             < descriptorSetsConfig[dsCounter]
                   .actualLinkedDescriptorBindingsNumber;
             ++DS_localBindingsCounter) {
            const u32 DS_sumBindingsCounter =
                DS_globalBindingsCounter + DS_localBindingsCounter;
            /// Global offset for discriptors inside ds binding
            descriptorBindingsConfig[DS_sumBindingsCounter]
                .globalDescriptorOffset = globalDescriptorsOffset;

            /// Index for ds bindings inside ds
            descriptorSetsConfig[dsCounter]
                .descriptorsBindingsIDs[DS_localBindingsCounter] =
                DS_sumBindingsCounter;
            if (descriptorBindingsConfig[DS_sumBindingsCounter].vkType
                == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) {
                for (unsigned int descriptorCounter = 0; descriptorCounter
                     < descriptorBindingsConfig[DS_sumBindingsCounter]
                           .shaderDescriptorsNumber;
                     ++descriptorCounter) {
                    GPUDescriptors.Push({});
                    GPUDescriptors[GPUDescriptors.GetSize() - 1].GPUBuffer =
                        new GPUBuffer;
                    ++globalDescriptorsOffset;
                }
            } else if (
                descriptorBindingsConfig[DS_sumBindingsCounter].vkType
                == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
            ) {
                for (unsigned int descriptorCounter = 0; descriptorCounter
                     < descriptorBindingsConfig[DS_sumBindingsCounter]
                           .shaderDescriptorsNumber;
                     ++descriptorCounter) {
                    GPUDescriptors.Push({});
                    GPUDescriptors[GPUDescriptors.GetSize() - 1].GPUImage =
                        new VK_Image;
                    ++globalDescriptorsOffset;
                }
            }
        }
        DS_globalBindingsCounter +=
            descriptorSetsConfig[dsCounter].actualLinkedDescriptorBindingsNumber;
    }
    descriptorSetsChunks.Resize(DS_hostNumber);
}

void pipelineBuilder() {
    static unsigned int descriptorSetsLayoutIdCounter = 0;
    for (unsigned int pipelineCounter = 0;
         pipelineCounter < SpecificPipeline::PIPELINES_NUMBER;
         ++pipelineCounter) {
        for (unsigned int linkedDSLayoutCounter = 0; linkedDSLayoutCounter
             < pipelineConfigs[pipelineCounter].actualLinkedDescriptorSetsNumber;
             ++linkedDSLayoutCounter) {
            pipelineConfigs[pipelineCounter]
                .linkedDescriptorSetIDs[linkedDSLayoutCounter] =
                descriptorSetsLayoutIdCounter + linkedDSLayoutCounter;
        }
        descriptorSetsLayoutIdCounter +=
            pipelineConfigs[pipelineCounter].actualLinkedDescriptorSetsNumber;
    }
}

void renderPassesBuilder() {
    renderPasses.Resize(SpecificPipeline::PIPELINES_NUMBER);
}
}; // namespace GLVM::core
