#include "glvm/graphic_api/render_config.hpp"
#include "glvm/graphic_api/render_data.hpp"
#include "glvm/vk_structs.hpp"
#include "glvm/typenames.hpp"

namespace glvm::core {
void descriptorSetBuilder() {
    // Counts ds bindings indexes inside ds.
    static unsigned int DS_globalBindingsCounter = 0;
    // Counts host data ds.
    static unsigned int DS_hostNumber = 0;
    // Counts offsets data descriptors.
    static unsigned int globalDescriptorsOffset = 0;

    for (unsigned int dsCounter = 0;
         dsCounter < DescriptorSetDataLink::DESCRIPTOR_CHUNKS_NUMBER;
         ++dsCounter) {
        // Offset for indexing inside descriptorSetsChunks.
        descriptorSetsConfig[dsCounter].descriptorSetOffset = DS_hostNumber;
        DS_hostNumber += descriptorSetsConfig[dsCounter].hostDescriptorNumber;

        for (unsigned int DS_localBindingsCounter = 0; DS_localBindingsCounter
             < descriptorSetsConfig[dsCounter]
                   .actualLinkedDescriptorBindingsNumber;
             ++DS_localBindingsCounter) {
            const uint32_t DS_sumBindingsCounter =
                DS_globalBindingsCounter + DS_localBindingsCounter;
            // Global offset for descriptors inside ds binding.
            descriptorBindingsConfig[DS_sumBindingsCounter]
                .globalDescriptorOffset = globalDescriptorsOffset;

            // Index for ds bindings inside ds.
            descriptorSetsConfig[dsCounter]
                .descriptorsBindingsIDs[DS_localBindingsCounter] =
                DS_sumBindingsCounter;
            if (descriptorBindingsConfig[DS_sumBindingsCounter].vkType
                == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) {
                for (unsigned int descriptorCounter = 0; descriptorCounter
                     < descriptorBindingsConfig[DS_sumBindingsCounter]
                           .shaderDescriptorsNumber;
                     ++descriptorCounter) {
                    GPUDescriptors.push_back({});
                    GPUDescriptors[GPUDescriptors.size() - 1].GPUBuffer =
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
                    GPUDescriptors.push_back({});
                    GPUDescriptors[GPUDescriptors.size() - 1].GPUImage =
                        new VK_Image;
                    ++globalDescriptorsOffset;
                }
            }
        }
        DS_globalBindingsCounter +=
            descriptorSetsConfig[dsCounter].actualLinkedDescriptorBindingsNumber;
    }
    descriptorSetsChunks.resize(DS_hostNumber);
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
    renderPasses.resize(SpecificPipeline::PIPELINES_NUMBER);
}
}; // namespace glvm::core
