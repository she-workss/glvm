#include "glvm/GraphicAPI/VkDebugUtils.hpp"

namespace GLVM::core::vkDebugUtils {
VkResult CreateDebugUtilsMessengerEXT(
    VkInstance instance,
    const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkDebugUtilsMessengerEXT* pDebugMessenger
) {
    static auto func = (PFN_vkCreateDebugUtilsMessengerEXT)
        vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void CreateBeginDebugUtilsLabelEXT(
    [[maybe_unused]] VkInstance instance,
    [[maybe_unused]] VkCommandBuffer commandBuffer,
    [[maybe_unused]] const VkDebugUtilsLabelEXT* labelInfo
) {
#ifndef NDEBUG
    static auto func = (PFN_vkCmdBeginDebugUtilsLabelEXT)
        vkGetInstanceProcAddr(instance, "vkCmdBeginDebugUtilsLabelEXT");
    func(commandBuffer, labelInfo);
#endif
}

void CreateEndDebugUtilsLabelEXT(
    [[maybe_unused]] VkInstance instance,
    [[maybe_unused]] VkCommandBuffer commandBuffer
) {
#ifndef NDEBUG
    static auto func = (PFN_vkCmdEndDebugUtilsLabelEXT)
        vkGetInstanceProcAddr(instance, "vkCmdEndDebugUtilsLabelEXT");
    func(commandBuffer);
#endif
}

void DestroyDebugUtilsMessengerEXT(
    VkInstance instance,
    VkDebugUtilsMessengerEXT debugMessenger,
    const VkAllocationCallbacks* pAllocator
) {
    static auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)
        vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}

VkResult SetDebugObjectName(
    VkDevice device,
    const VkDebugUtilsObjectNameInfoEXT* objectNameInfo
) {
    static auto func = (PFN_vkSetDebugUtilsObjectNameEXT)
        vkGetDeviceProcAddr(device, "vkSetDebugUtilsObjectNameEXT");
    if (func != nullptr) {
        return func(device, objectNameInfo);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void setImageDebugObjectName(
    VkDevice device,
    VK_Image image,
    std::string imageName
) {
    VkDebugUtilsObjectNameInfoEXT imageObjectInfo {};
    imageObjectInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    std::string imageName1 = ConcatIntBetweenTwoStrings(
        VK_DEBUG_IMAGE_SET_RED,
        " \x1b[31m" + imageName + " pipeline #\x1b[0m ",
        0
    );
    const char* strImageName = imageName1.c_str();
    imageObjectInfo.pObjectName = strImageName;
    imageObjectInfo.objectType = VK_OBJECT_TYPE_IMAGE;
    imageObjectInfo.objectHandle = (uint64_t)image.image;
    SetDebugObjectName(device, &imageObjectInfo);
}

void setPipelineDebugObjectName(
    VkDevice device,
    VkPipeline pipeline,
    std::string pipelineName
) {
    VkDebugUtilsObjectNameInfoEXT mainPipelineObjectInfo {};
    mainPipelineObjectInfo.sType =
        VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    std::string mainPipeLineImageName = ConcatIntBetweenTwoStrings(
        VK_DEBUG_PIPELINE_RED,
        " \x1b[31m" + pipelineName + " pipeline #\x1b[0m ",
        0
    );
    const char* mainPipeLineStrImageName = mainPipeLineImageName.c_str();
    mainPipelineObjectInfo.pObjectName = mainPipeLineStrImageName;
    mainPipelineObjectInfo.objectType = VK_OBJECT_TYPE_PIPELINE;
    mainPipelineObjectInfo.objectHandle = (uint64_t)pipeline;
    SetDebugObjectName(device, &mainPipelineObjectInfo);
}

void setDescriptorSetObjectName(
    VkDevice device,
    VkDescriptorSet descriptorSet,
    std::string descriptorSetName,
    unsigned int index
) {
    VkDebugUtilsObjectNameInfoEXT descriptorSetObjectInfo {};
    descriptorSetObjectInfo.sType =
        VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    std::string name = ConcatIntBetweenTwoStrings(
        VK_DEBUG_DESCRIPTOR_SET_RED,
        " \x1b[31m" + descriptorSetName + " descriptor set #\x1b[0m ",
        index
    );
    const char* strName = name.c_str();
    descriptorSetObjectInfo.pObjectName = strName;
    descriptorSetObjectInfo.objectType = VK_OBJECT_TYPE_DESCRIPTOR_SET;
    descriptorSetObjectInfo.objectHandle = (uint64_t)descriptorSet;
    SetDebugObjectName(device, &descriptorSetObjectInfo);
}

void setDebugObjectNames(
    VkDevice device,
    const std::vector<VkBuffer>& vertexBufferContainer,
    const std::vector<VkBuffer>& indexBufferContainer,
    const GLVM::core::vector<Descriptor>& GPUDescriptors,
    const std::vector<unsigned int>& fontIndicesContainer,
    const std::vector<VkBuffer>& fontVertexBufferContainer,
    const std::vector<VkBuffer>& fontIndexBufferContainer
) {
    setPipelineDebugObjectName(
        device,
        pipelineConfigs[SpecificPipeline::FONT_PIPELINE].pipeline,
        "fontPipeline"
    );
    setPipelineDebugObjectName(
        device,
        pipelineConfigs[SpecificPipeline::UI_PIPELINE].pipeline,
        "uiPipeline"
    );
    setPipelineDebugObjectName(
        device,
        pipelineConfigs[SpecificPipeline::UI_ICONS_PIPELINE].pipeline,
        "uiIconsPipeline"
    );

    VkDebugUtilsObjectNameInfoEXT mainPipelineObjectInfo {};
    mainPipelineObjectInfo.sType =
        VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    std::string mainPipeLineImageName = ConcatIntBetweenTwoStrings(
        VK_DEBUG_PIPELINE_RED,
        " \x1b[31mMain pipeline #\x1b[0m ",
        0
    );
    const char* mainPipeLineStrImageName = mainPipeLineImageName.c_str();
    mainPipelineObjectInfo.pObjectName = mainPipeLineStrImageName;
    mainPipelineObjectInfo.objectType = VK_OBJECT_TYPE_PIPELINE;
    mainPipelineObjectInfo.objectHandle =
        (uint64_t)pipelineConfigs[SpecificPipeline::MAIN_RENDER_PIPELINE]
            .pipeline;
    SetDebugObjectName(device, &mainPipelineObjectInfo);

    VkDebugUtilsObjectNameInfoEXT mainPipelineLayoutObjectInfo {};
    mainPipelineLayoutObjectInfo.sType =
        VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    std::string mainPipelineLayoutImageName = ConcatIntBetweenTwoStrings(
        VK_DEBUG_PIPELINE_LAYOUT_RED,
        " \x1b[31mMain pipeline layout #\x1b[0m ",
        0
    );
    const char* mainPipelineLayoutStrImageName =
        mainPipelineLayoutImageName.c_str();
    mainPipelineLayoutObjectInfo.pObjectName = mainPipelineLayoutStrImageName;
    mainPipelineLayoutObjectInfo.objectType = VK_OBJECT_TYPE_PIPELINE_LAYOUT;
    mainPipelineLayoutObjectInfo.objectHandle =
        (uint64_t)pipelineConfigs[SpecificPipeline::MAIN_RENDER_PIPELINE]
            .pipelineLayout;
    SetDebugObjectName(device, &mainPipelineObjectInfo);

    VkDebugUtilsObjectNameInfoEXT directionalLightPipelineObjectInfo {};
    directionalLightPipelineObjectInfo.sType =
        VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    std::string directionalLightPipeLineImageName = ConcatIntBetweenTwoStrings(
        VK_DEBUG_PIPELINE_RED,
        " \x1b[31mDirectional light pipeline #\x1b[0m ",
        0
    );
    const char* directionalLightPipeLineStrImageName =
        directionalLightPipeLineImageName.c_str();
    directionalLightPipelineObjectInfo.pObjectName =
        directionalLightPipeLineStrImageName;
    directionalLightPipelineObjectInfo.objectType = VK_OBJECT_TYPE_PIPELINE;
    directionalLightPipelineObjectInfo.objectHandle =
        (uint64_t)pipelineConfigs[SpecificPipeline::DIRECTIONAL_LIGHT_PIPELINE]
            .pipeline;
    SetDebugObjectName(device, &directionalLightPipelineObjectInfo);

    VkDebugUtilsObjectNameInfoEXT directionalLightPipelineLayoutObjectInfo {};
    directionalLightPipelineLayoutObjectInfo.sType =
        VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    std::string directionalLightPipelineLayoutImageName =
        ConcatIntBetweenTwoStrings(
            VK_DEBUG_PIPELINE_LAYOUT_RED,
            " \x1b[31mDirectional light pipeline layout #\x1b[0m ",
            0
        );
    const char* directionalLightPipelineLayoutStrImageName =
        directionalLightPipelineLayoutImageName.c_str();
    directionalLightPipelineLayoutObjectInfo.pObjectName =
        directionalLightPipelineLayoutStrImageName;
    directionalLightPipelineLayoutObjectInfo.objectType =
        VK_OBJECT_TYPE_PIPELINE_LAYOUT;
    directionalLightPipelineLayoutObjectInfo.objectHandle =
        (uint64_t)pipelineConfigs[SpecificPipeline::DIRECTIONAL_LIGHT_PIPELINE]
            .pipelineLayout;
    SetDebugObjectName(device, &directionalLightPipelineLayoutObjectInfo);

    VkDebugUtilsObjectNameInfoEXT spotLightPipelineObjectInfo {};
    spotLightPipelineObjectInfo.sType =
        VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    std::string spotLightPipeLineImageName = ConcatIntBetweenTwoStrings(
        VK_DEBUG_PIPELINE_RED,
        " \x1b[31mSpot light pipeline #\x1b[0m ",
        0
    );
    const char* spotLightPipeLineStrImageName =
        spotLightPipeLineImageName.c_str();
    spotLightPipelineObjectInfo.pObjectName = spotLightPipeLineStrImageName;
    spotLightPipelineObjectInfo.objectType = VK_OBJECT_TYPE_PIPELINE;
    spotLightPipelineObjectInfo.objectHandle =
        (uint64_t)pipelineConfigs[SpecificPipeline::SPOT_LIGHT_PIPELINE]
            .pipeline;
    SetDebugObjectName(device, &spotLightPipelineObjectInfo);

    VkDebugUtilsObjectNameInfoEXT spotLightPipelineLayoutObjectInfo {};
    spotLightPipelineLayoutObjectInfo.sType =
        VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    std::string spotLightPipelineLayoutImageName = ConcatIntBetweenTwoStrings(
        VK_DEBUG_PIPELINE_LAYOUT_RED,
        " \x1b[31mSpot light pipeline layout #\x1b[0m ",
        0
    );
    const char* spotLightPipelineLayoutStrImageName =
        spotLightPipelineLayoutImageName.c_str();
    spotLightPipelineLayoutObjectInfo.pObjectName =
        spotLightPipelineLayoutStrImageName;
    spotLightPipelineLayoutObjectInfo.objectType =
        VK_OBJECT_TYPE_PIPELINE_LAYOUT;
    spotLightPipelineLayoutObjectInfo.objectHandle =
        (uint64_t)pipelineConfigs[SpecificPipeline::SPOT_LIGHT_PIPELINE]
            .pipelineLayout;
    SetDebugObjectName(device, &spotLightPipelineLayoutObjectInfo);

    VkDebugUtilsObjectNameInfoEXT pointLightPipelineObjectInfo {};
    pointLightPipelineObjectInfo.sType =
        VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    std::string pointLightPipeLineImageName = ConcatIntBetweenTwoStrings(
        VK_DEBUG_PIPELINE_RED,
        " \x1b[31mPoint light pipeline #\x1b[0m ",
        0
    );
    const char* pointLightPipeLineStrImageName =
        pointLightPipeLineImageName.c_str();
    pointLightPipelineObjectInfo.pObjectName = pointLightPipeLineStrImageName;
    pointLightPipelineObjectInfo.objectType = VK_OBJECT_TYPE_PIPELINE;
    pointLightPipelineObjectInfo.objectHandle =
        (uint64_t)pipelineConfigs[SpecificPipeline::POINT_LIGHT_PIPELINE]
            .pipeline;
    SetDebugObjectName(device, &pointLightPipelineObjectInfo);

    VkDebugUtilsObjectNameInfoEXT pointLightPipelineLayoutObjectInfo {};
    pointLightPipelineLayoutObjectInfo.sType =
        VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    std::string pointLightPipelineLayoutImageName = ConcatIntBetweenTwoStrings(
        VK_DEBUG_PIPELINE_LAYOUT_RED,
        " \x1b[31mPoint light pipeline layout #\x1b[0m ",
        0
    );
    const char* pointLightPipelineLayoutStrImageName =
        pointLightPipelineLayoutImageName.c_str();
    pointLightPipelineLayoutObjectInfo.pObjectName =
        pointLightPipelineLayoutStrImageName;
    pointLightPipelineLayoutObjectInfo.objectType =
        VK_OBJECT_TYPE_PIPELINE_LAYOUT;
    pointLightPipelineLayoutObjectInfo.objectHandle =
        (uint64_t)pipelineConfigs[SpecificPipeline::POINT_LIGHT_PIPELINE]
            .pipelineLayout;
    SetDebugObjectName(device, &pointLightPipelineLayoutObjectInfo);

    VkDebugUtilsObjectNameInfoEXT hudUniformBufferObjectInfo {};
    hudUniformBufferObjectInfo.sType =
        VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    std::string hudImageName = ConcatIntBetweenTwoStrings(
        VK_DEBUG_IMAGE_SET_RED,
        " Hud uniform buffer # ",
        0
    );
    const char* hudStrImageName = hudImageName.c_str();
    hudUniformBufferObjectInfo.pObjectName = hudStrImageName;
    hudUniformBufferObjectInfo.objectType = VK_OBJECT_TYPE_BUFFER;
    unsigned int hudUboDescriptorBindingIndex =
        descriptorSetsConfig[DescriptorSetDataLink::HUD]
            .descriptorsBindingsIDs[0];
    hudUniformBufferObjectInfo.objectHandle =
        (uint64_t)
            GPUDescriptors[descriptorBindingsConfig[hudUboDescriptorBindingIndex]
                               .globalDescriptorOffset]
                .GPUBuffer->buffer;
    SetDebugObjectName(device, &hudUniformBufferObjectInfo);

    VkDebugUtilsObjectNameInfoEXT fontUniformBufferObjectInfo {};
    fontUniformBufferObjectInfo.sType =
        VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    std::string fontImageName = ConcatIntBetweenTwoStrings(
        VK_DEBUG_IMAGE_SET_RED,
        " Font uniform buffer # ",
        0
    );
    const char* fontStrImageName = fontImageName.c_str();
    fontUniformBufferObjectInfo.pObjectName = fontStrImageName;
    fontUniformBufferObjectInfo.objectType = VK_OBJECT_TYPE_BUFFER;
    unsigned int fontUboDescriptorBindingIndex =
        descriptorSetsConfig[DescriptorSetDataLink::FONT_RENDER_UBO]
            .descriptorsBindingsIDs[0];
    fontUniformBufferObjectInfo.objectHandle =
        (uint64_t)
            GPUDescriptors[descriptorBindingsConfig[fontUboDescriptorBindingIndex]
                               .globalDescriptorOffset]
                .GPUBuffer->buffer;
    ;
    SetDebugObjectName(device, &fontUniformBufferObjectInfo);

    VkDebugUtilsObjectNameInfoEXT uiUniformBufferObjectInfo {};
    uiUniformBufferObjectInfo.sType =
        VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    std::string uiImageName = ConcatIntBetweenTwoStrings(
        VK_DEBUG_IMAGE_SET_RED,
        " UI uniform buffer # ",
        0
    );
    const char* uiStrImageName = uiImageName.c_str();
    uiUniformBufferObjectInfo.pObjectName = uiStrImageName;
    uiUniformBufferObjectInfo.objectType = VK_OBJECT_TYPE_BUFFER;
    unsigned int uiUboDescriptorBindingIndex =
        descriptorSetsConfig[DescriptorSetDataLink::UI]
            .descriptorsBindingsIDs[0];
    uiUniformBufferObjectInfo.objectHandle =
        (uint64_t)
            GPUDescriptors[descriptorBindingsConfig[uiUboDescriptorBindingIndex]
                               .globalDescriptorOffset]
                .GPUBuffer->buffer;
    SetDebugObjectName(device, &uiUniformBufferObjectInfo);

    VkDebugUtilsObjectNameInfoEXT uiIconsUniformBufferObjectInfo {};
    uiIconsUniformBufferObjectInfo.sType =
        VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    std::string uiIconsImageName = ConcatIntBetweenTwoStrings(
        VK_DEBUG_IMAGE_SET_RED,
        " UI icons uniform buffer # ",
        0
    );
    const char* uiIconsStrImageName = uiIconsImageName.c_str();
    uiIconsUniformBufferObjectInfo.pObjectName = uiIconsStrImageName;
    uiIconsUniformBufferObjectInfo.objectType = VK_OBJECT_TYPE_BUFFER;
    unsigned int uiIconsUboDescriptorBindingIndex =
        descriptorSetsConfig[DescriptorSetDataLink::UI_ICONS]
            .descriptorsBindingsIDs[0];
    uiIconsUniformBufferObjectInfo.objectHandle =
        (uint64_t)GPUDescriptors
            [descriptorBindingsConfig[uiIconsUboDescriptorBindingIndex]
                 .globalDescriptorOffset]
                .GPUBuffer->buffer;
    SetDebugObjectName(device, &uiIconsUniformBufferObjectInfo);

    VkDebugUtilsObjectNameInfoEXT directionalLightUniformBufferObjectInfo {};
    directionalLightUniformBufferObjectInfo.sType =
        VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    std::string directionalLightImageName = ConcatIntBetweenTwoStrings(
        VK_DEBUG_IMAGE_SET_RED,
        " Shadow map directional light model matrix uniform buffer # ",
        0
    );
    const char* directionalLightStrImageName =
        directionalLightImageName.c_str();
    directionalLightUniformBufferObjectInfo.pObjectName =
        directionalLightStrImageName;
    directionalLightUniformBufferObjectInfo.objectType = VK_OBJECT_TYPE_BUFFER;
    unsigned int shadowMapDirectionalLightDescriptorBindingIndex =
        descriptorSetsConfig[DescriptorSetDataLink::SHADOW_MAP_DIRECTIONAL_LIGHT]
            .descriptorsBindingsIDs[0];
    directionalLightUniformBufferObjectInfo.objectHandle =
        (uint64_t)
            GPUDescriptors[descriptorBindingsConfig
                               [shadowMapDirectionalLightDescriptorBindingIndex]
                                   .globalDescriptorOffset]
                .GPUBuffer->buffer;
    SetDebugObjectName(device, &directionalLightUniformBufferObjectInfo);

    VkDebugUtilsObjectNameInfoEXT pointLightUniformBufferObjectInfo {};
    pointLightUniformBufferObjectInfo.sType =
        VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    std::string pointLightImageName = ConcatIntBetweenTwoStrings(
        VK_DEBUG_IMAGE_SET_RED,
        " Shadow map point light model matrix uniform buffer # ",
        0
    );
    const char* pointLightStrImageName = pointLightImageName.c_str();
    pointLightUniformBufferObjectInfo.pObjectName = pointLightStrImageName;
    pointLightUniformBufferObjectInfo.objectType = VK_OBJECT_TYPE_BUFFER;
    unsigned int shadowMapPointLightDescriptorBindingIndex =
        descriptorSetsConfig[DescriptorSetDataLink::SHADOW_MAP_POINT_LIGHT]
            .descriptorsBindingsIDs[0];
    pointLightUniformBufferObjectInfo.objectHandle =
        (uint64_t)GPUDescriptors
            [descriptorBindingsConfig[shadowMapPointLightDescriptorBindingIndex]
                 .globalDescriptorOffset]
                .GPUBuffer->buffer;
    SetDebugObjectName(device, &pointLightUniformBufferObjectInfo);

    VkDebugUtilsObjectNameInfoEXT spotLightUniformBufferObjectInfo {};
    spotLightUniformBufferObjectInfo.sType =
        VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    std::string spotLightImageName = ConcatIntBetweenTwoStrings(
        VK_DEBUG_IMAGE_SET_RED,
        " Shadow map spot light model matrix uniform buffer # ",
        0
    );
    const char* spotLightStrImageName = spotLightImageName.c_str();
    spotLightUniformBufferObjectInfo.pObjectName = spotLightStrImageName;
    spotLightUniformBufferObjectInfo.objectType = VK_OBJECT_TYPE_BUFFER;
    unsigned int shadowMapSpotLightDescriptorBindingIndex =
        descriptorSetsConfig[DescriptorSetDataLink::SHADOW_MAP_SPOT_LIGHT]
            .descriptorsBindingsIDs[0];
    spotLightUniformBufferObjectInfo.objectHandle =
        (uint64_t)GPUDescriptors
            [descriptorBindingsConfig[shadowMapSpotLightDescriptorBindingIndex]
                 .globalDescriptorOffset]
                .GPUBuffer->buffer;
    SetDebugObjectName(device, &spotLightUniformBufferObjectInfo);

    for (unsigned long i = 0; i < vertexBufferContainer.size(); ++i) {
        VkDebugUtilsObjectNameInfoEXT uniformBufferObjectInfo {};
        uniformBufferObjectInfo.sType =
            VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
        std::string imageName = ConcatIntBetweenTwoStrings(
            VK_DEBUG_IMAGE_SET_RED,
            " Vertex uniform buffer # ",
            i
        );
        const char* strImageName = imageName.c_str();
        uniformBufferObjectInfo.pObjectName = strImageName;
        uniformBufferObjectInfo.objectType = VK_OBJECT_TYPE_BUFFER;
        uniformBufferObjectInfo.objectHandle =
            (uint64_t)vertexBufferContainer[i];
        SetDebugObjectName(device, &uniformBufferObjectInfo);
    }

    for (unsigned long i = 0; i < indexBufferContainer.size(); ++i) {
        VkDebugUtilsObjectNameInfoEXT uniformBufferObjectInfo {};
        uniformBufferObjectInfo.sType =
            VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
        std::string imageName = ConcatIntBetweenTwoStrings(
            VK_DEBUG_IMAGE_SET_RED,
            " Index uniform buffer # ",
            i
        );
        const char* strImageName = imageName.c_str();
        uniformBufferObjectInfo.pObjectName = strImageName;
        uniformBufferObjectInfo.objectType = VK_OBJECT_TYPE_BUFFER;
        uniformBufferObjectInfo.objectHandle =
            (uint64_t)indexBufferContainer[i];
        SetDebugObjectName(device, &uniformBufferObjectInfo);
    }

    for (unsigned long i = 0; i < fontIndicesContainer.size(); ++i) {
        VkDebugUtilsObjectNameInfoEXT uniformBufferObjectInfo {};
        uniformBufferObjectInfo.sType =
            VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
        std::string imageName = ConcatIntBetweenTwoStrings(
            VK_DEBUG_IMAGE_SET_RED,
            " Font vertex uniform buffer # ",
            fontIndicesContainer[i]
        );
        const char* strImageName = imageName.c_str();
        uniformBufferObjectInfo.pObjectName = strImageName;
        uniformBufferObjectInfo.objectType = VK_OBJECT_TYPE_BUFFER;
        uniformBufferObjectInfo.objectHandle =
            (uint64_t)fontVertexBufferContainer[fontIndicesContainer[i]];
        SetDebugObjectName(device, &uniformBufferObjectInfo);
    }

    for (unsigned long i = 0; i < fontIndicesContainer.size(); ++i) {
        VkDebugUtilsObjectNameInfoEXT uniformBufferObjectInfo {};
        uniformBufferObjectInfo.sType =
            VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
        std::string imageName = ConcatIntBetweenTwoStrings(
            VK_DEBUG_IMAGE_SET_RED,
            " Font index uniform buffer # ",
            fontIndicesContainer[i]
        );
        const char* strImageName = imageName.c_str();
        uniformBufferObjectInfo.pObjectName = strImageName;
        uniformBufferObjectInfo.objectType = VK_OBJECT_TYPE_BUFFER;
        uniformBufferObjectInfo.objectHandle =
            (uint64_t)fontIndexBufferContainer[fontIndicesContainer[i]];
        SetDebugObjectName(device, &uniformBufferObjectInfo);
    }

    VkDebugUtilsObjectNameInfoEXT uniformBufferObjectInfo {};
    uniformBufferObjectInfo.sType =
        VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    std::string imageName = ConcatIntBetweenTwoStrings(
        VK_DEBUG_IMAGE_SET_RED,
        " Model matrix uniform buffer # ",
        0
    );
    const char* strImageName = imageName.c_str();
    uniformBufferObjectInfo.pObjectName = strImageName;
    uniformBufferObjectInfo.objectType = VK_OBJECT_TYPE_BUFFER;
    uniformBufferObjectInfo.objectHandle =
        (uint64_t)GPUDescriptors[DescriptorSetDataLink::MAIN_RENDER_MATRIX_UBO]
            .GPUBuffer->buffer;
    SetDebugObjectName(device, &uniformBufferObjectInfo);

    VkDebugUtilsObjectNameInfoEXT lightDataUniformBufferObjectInfo {};
    lightDataUniformBufferObjectInfo.sType =
        VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    std::string lightDataImageName = ConcatIntBetweenTwoStrings(
        VK_DEBUG_IMAGE_SET_RED,
        " Light data uniform buffer # ",
        0
    );
    const char* lightDataStrImageName = lightDataImageName.c_str();
    lightDataUniformBufferObjectInfo.pObjectName = lightDataStrImageName;
    lightDataUniformBufferObjectInfo.objectType = VK_OBJECT_TYPE_BUFFER;
    unsigned int lightDataUboDescriptorBindingIndex =
        descriptorSetsConfig[DescriptorSetDataLink::MAIN_RENDER_LIGHT_DATA_UBO]
            .descriptorsBindingsIDs[0];
    lightDataUniformBufferObjectInfo.objectHandle =
        (uint64_t)GPUDescriptors
            [descriptorBindingsConfig[lightDataUboDescriptorBindingIndex]
                 .globalDescriptorOffset]
                .GPUBuffer->buffer;
    SetDebugObjectName(device, &lightDataUniformBufferObjectInfo);

    // for ( unsigned long i = 0; i <
    // directionalLightPipeline.descriptors[0].textureImages.size(); ++i ) {
    // 	VkDebugUtilsObjectNameInfoEXT directionalLightImageObjectInfo{};
    // 	directionalLightImageObjectInfo.sType =
    // VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT; 	std::string
    // imageName = ConcatIntBetweenTwoStrings(VK_DEBUG_IMAGE_SET_RED, "
    // Directional light shadow map image # ", i); 	const char* strImageName =
    // imageName.c_str(); 	directionalLightImageObjectInfo.pObjectName =
    // strImageName; 	directionalLightImageObjectInfo.objectType =
    // VK_OBJECT_TYPE_IMAGE; 	directionalLightImageObjectInfo.objectHandle =
    // (uint64_t)directionalLightPipeline.descriptors[0].textureImages[i].image;
    // 	SetDebugObjectName(device, &directionalLightImageObjectInfo);
    // }

    // for ( unsigned long i = 0; i < textureImages.size(); ++i ) {
    // 	VkDebugUtilsObjectNameInfoEXT textureImageObjectInfo{};
    // 	textureImageObjectInfo.sType =
    // VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT; 	std::string
    // imageName = ConcatIntBetweenTwoStrings(VK_DEBUG_IMAGE_SET_RED, " Texture
    // image # ", i); 	const char* strImageName = imageName.c_str();
    // 	textureImageObjectInfo.pObjectName = strImageName;
    // 	textureImageObjectInfo.objectType = VK_OBJECT_TYPE_IMAGE;
    // 	textureImageObjectInfo.objectHandle = (uint64_t)textureImages[i].image;
    // 	SetDebugObjectName(device, &textureImageObjectInfo);
    // }

    // for ( unsigned long i = 0; i < swapChainImages.size(); ++i ) {
    // 	VkDebugUtilsObjectNameInfoEXT swapChainImageObjectInfo{};
    // 	swapChainImageObjectInfo.sType =
    // VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT; 	std::string imageName
    // = ConcatIntBetweenTwoStrings(VK_DEBUG_IMAGE_SET_RED, " SwapChain image #
    // ", i); 	const char* strImageName = imageName.c_str();
    // 	swapChainImageObjectInfo.pObjectName = strImageName;
    // 	swapChainImageObjectInfo.objectType = VK_OBJECT_TYPE_IMAGE;
    // 	swapChainImageObjectInfo.objectHandle = (uint64_t)swapChainImages[i];
    // 	SetDebugObjectName(device, &swapChainImageObjectInfo);
    // }

    // for ( unsigned long i = 0; i <
    // directionalLightPipeline.descriptors.GetSize(); ++i ) {
    // 	VkDebugUtilsObjectNameInfoEXT descriptorSetLayoutObjectInfo{};
    // 	descriptorSetLayoutObjectInfo.sType =
    // VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT; 	std::string
    // layoutName =
    // ConcatIntBetweenTwoStrings(VK_DEBUG_DESCRIPTOR_SET_LAYOUT_RED, "
    // Directional light shadow map descriptor set layout # ", i); 	const char*
    // strLayoutName = layoutName.c_str();
    // descriptorSetLayoutObjectInfo.pObjectName = strLayoutName;
    // descriptorSetLayoutObjectInfo.objectType =
    // VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT;
    // 	descriptorSetLayoutObjectInfo.objectHandle =
    // (uint64_t)directionalLightPipeline.descriptors[i].setLayout;
    // 	SetDebugObjectName(device, &descriptorSetLayoutObjectInfo);
    // }

    // for ( unsigned long i = 0; i <
    // mainRenderScenePipeline.descriptors.GetSize(); ++i ) {
    // 	VkDebugUtilsObjectNameInfoEXT descriptorSetLayoutObjectInfo{};
    // 	descriptorSetLayoutObjectInfo.sType =
    // VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT; 	std::string
    // layoutName =
    // ConcatIntBetweenTwoStrings(VK_DEBUG_DESCRIPTOR_SET_LAYOUT_RED, " Main
    // render descriptor set layout # ", i); 	const char* strLayoutName =
    // layoutName.c_str(); 	descriptorSetLayoutObjectInfo.pObjectName =
    // strLayoutName; 	descriptorSetLayoutObjectInfo.objectType =
    // VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT;
    // 	descriptorSetLayoutObjectInfo.objectHandle =
    // (uint64_t)mainRenderScenePipeline.descriptors[i].setLayout;
    // 	SetDebugObjectName(device, &descriptorSetLayoutObjectInfo);
    // }

    // for ( unsigned long i = 0; i < viewPositionUboDescriptorSets.size(); ++i
    // ) { 	VkDebugUtilsObjectNameInfoEXT descriptorSetObjectInfo{};
    // 	descriptorSetObjectInfo.sType =
    // VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT; 	std::string name =
    // ConcatIntBetweenTwoStrings(VK_DEBUG_DESCRIPTOR_SET_RED, " Main render
    // view position descriptor set # ", i); 	const char* strName =
    // name.c_str(); 	descriptorSetObjectInfo.pObjectName = strName;
    // 	descriptorSetObjectInfo.objectType = VK_OBJECT_TYPE_DESCRIPTOR_SET;
    // 	descriptorSetObjectInfo.objectHandle =
    // (uint64_t)viewPositionUboDescriptorSets[i]; 	SetDebugObjectName(device,
    // &descriptorSetObjectInfo);
    // }

    // for ( unsigned long i = 0; i <
    // shadowMapDirectionalLightDescriptorSets.size(); ++i ) {
    // 	VkDebugUtilsObjectNameInfoEXT descriptorSetObjectInfo{};
    // 	descriptorSetObjectInfo.sType =
    // VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT; 	std::string name =
    // ConcatIntBetweenTwoStrings(VK_DEBUG_DESCRIPTOR_SET_RED, " Main render
    // shadow map directional light descriptor set # ", i); 	const char*
    // strName = name.c_str(); 	descriptorSetObjectInfo.pObjectName = strName;
    // 	descriptorSetObjectInfo.objectType = VK_OBJECT_TYPE_DESCRIPTOR_SET;
    // 	descriptorSetObjectInfo.objectHandle =
    // (uint64_t)shadowMapDirectionalLightDescriptorSets[i];
    // 	SetDebugObjectName(device, &descriptorSetObjectInfo);
    // }
}
}; // namespace GLVM::core::vkDebugUtils
