#pragma once

#include "glvm/shader_structs.hpp"
#include "glvm/wavefront_obj_parser.hpp"

#include <cfloat>
#include <vulkan/vulkan_core.h>

namespace glvm::core {
struct MeshAxisMaxAbsoluteValues {
    float absolute_x = 0.0f;
    float absolute_y = 0.0f;
    float absolute_z = 0.0f;

    float origin_offset_x = 0.0f;
    float origin_offset_y = 0.0f;
    float origin_offset_z = 0.0f;
};

struct MeshAxisLimitingValues {
    float lowest_x = FLT_MAX;
    float highest_x = -FLT_MAX;
    float lowest_y = FLT_MAX;
    float highest_y = -FLT_MAX;
    float lowest_z = FLT_MAX;
    float highest_z = -FLT_MAX;

    void setToDefaultValues() {
        highest_x = -FLT_MAX;
        lowest_x = FLT_MAX;
        highest_y = -FLT_MAX;
        lowest_y = FLT_MAX;
        highest_z = -FLT_MAX;
        lowest_z = FLT_MAX;
    }

    void comparePerDirectionAndSetToMaximumValueByModule(SVertex& vertex) {
        if (vertex[0] < lowest_x) {
            lowest_x = vertex[0];
        } else if (vertex[0] > highest_x) {
            highest_x = vertex[0];
        }

        if (vertex[1] < lowest_y) {
            lowest_y = vertex[1];
        } else if (vertex[1] > highest_y) {
            highest_y = vertex[1];
        }

        if (vertex[2] < lowest_z) {
            lowest_z = vertex[2];
        } else if (vertex[2] > highest_z) {
            highest_z = vertex[2];
        }
    }

    void comparePerDirectionAndSetToMaximumValueByModule(
        Vector<float, 3> position,
        float half_x,
        float half_y,
        float half_z
    ) {
        if (position[0] + half_x > highest_x) {
            highest_x = position[0] + half_x;
        }
        if (position[0] - half_x < lowest_x) {
            lowest_x = position[0] - half_x;
        }
        if (position[1] + half_y > highest_y) {
            highest_y = position[1] + half_y;
        }
        if (position[1] - half_y < lowest_y) {
            lowest_y = position[1] - half_y;
        }
        if (position[2] + half_z > highest_z) {
            highest_z = position[2] + half_z;
        }
        if (position[2] - half_z < lowest_z) {
            lowest_z = position[2] - half_z;
        }
    }
};

enum DescriptorSetDataLink {
    // Pipelines related values.
    SHADOW_MAP_DIRECTIONAL_LIGHT,
    SHADOW_MAP_SPOT_LIGHT,
    SHADOW_MAP_POINT_LIGHT,
    HUD,
    FONT_RENDER_UBO,
    FONT_RENDER_SAMPLER,
    HUD_SCREEN,
    UI,
    UI_SAMPLERS,
    UI_ICONS,
    UI_ICONS_SAMPLERS,
    VIRTUAL_TEXTURES_UBO,
    VIRTUAL_TEXTURES_TILESET,
    MAIN_RENDER_MATRIX_UBO,
    MAIN_RENDER_LIGHT_DATA_UBO,
    MAIN_RENDER_SPECULAR_SAMPLER,
    MAIN_RENDER_DIFFUSE_SAMPLER,
    SDF_DATA,
    // Not related to any pipeline values.
    RIDABLE_TEXTURES,
    DESCRIPTOR_CHUNKS_NUMBER
};

enum SpecificPipeline {
    DIRECTIONAL_LIGHT_PIPELINE,
    SPOT_LIGHT_PIPELINE,
    POINT_LIGHT_PIPELINE,
    HUD_PIPELINE,
    FONT_PIPELINE,
    HUD_SCREEN_PIPELINE,
    UI_PIPELINE,
    UI_ICONS_PIPELINE,
    VIRTUAL_TEXTURES_PIPELINE,
    MAIN_RENDER_PIPELINE,
    SDF_PIPELINE,
    PIPELINES_NUMBER
};

struct RenderPass {
    unsigned int actualAttachmentDescriptionNumber;
    VkAttachmentDescription attachmentDescriptions[16];
    unsigned int actualAttachmentReferenceNumber;
    VkAttachmentReference attachmentReferences[16];
    unsigned int actualSubpassDependencyNumber;
    VkSubpassDependency subpassDependencies[8];
};

struct VK_Image {
    VkImage image;
    VkDeviceMemory deviceMemory = {};
    std::vector<VkImageView> views = {};
    VkImageViewType viewType = {};
    VkImageCreateFlags createFlags = {};
    VkMemoryPropertyFlags memoryPropertyFlags = {};
    VkImageUsageFlags usageFlags = {};
    VkImageAspectFlags aspectFlags = {};
    VkFormat format = {};
    VkImageTiling tiling = {};
    VkSampler sampler = {};
    VkComponentSwizzle red = {};
    VkComponentSwizzle green = {};
    VkComponentSwizzle blue = {};
    VkComponentSwizzle alpha = {};
    uint32_t arrayLayers = 0;
    uint32_t width = 0;
    uint32_t height = 0;
};

// Metadata for descriptor bindings.
struct DescriptorBinding {
    VkDescriptorType vkType;
    VkShaderStageFlags shaderStageFlag;
    unsigned int binding;
    unsigned int shaderDescriptorsNumber;
    unsigned int globalDescriptorOffset;
    VkDeviceSize uboChunkSize;
};

// Metadata for descriptor sets.
struct DescriptorSet {
    unsigned int actualLinkedDescriptorBindingsNumber;
    unsigned int hostDescriptorNumber;
    VkDescriptorSetLayout setLayout;
    static constexpr unsigned int maximumLinkedDescriptorBindingsDS = 32;
    unsigned int descriptorsBindingsIDs[maximumLinkedDescriptorBindingsDS];
    unsigned int descriptorSetOffset;
    bool isTexture;
};

struct Pipeline {
    VkPipeline pipeline;
    VkPipelineLayout pipelineLayout;
    const char* vertShader = nullptr;
    const char* fragShader = nullptr;
    VkVertexInputBindingDescription bindingDescription;
    std::array<VkVertexInputAttributeDescription, 5> attributeDescriptions;
    unsigned int actualLinkedDescriptorSetsNumber;
    static constexpr unsigned int maximumLinkedDescriptorSetDS = 32;
    unsigned int linkedDescriptorSetIDs[maximumLinkedDescriptorSetDS];
};

struct GPUBuffer {
    VkBuffer buffer;
    VkDeviceMemory deviceMemory;
};

union Descriptor {
    Descriptor() {};
    ~Descriptor() {};

    GPUBuffer* GPUBuffer;
    VK_Image* GPUImage;
};

struct Vertex {
    Vector<float, 3> pos;
    Vector<float, 3> color;
    Vector<float, 2> texCoord;
    Vector<float, 4> joinIndices;
    Vector<float, 4> weights;

    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription bindingDescription {};
        bindingDescription.binding = 0;
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        bindingDescription.stride = sizeof(Vertex);

        return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 5>
    getAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 5>
            attributeDescriptions {};

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, pos);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, color);

        attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

        attributeDescriptions[3].binding = 0;
        attributeDescriptions[3].location = 3;
        attributeDescriptions[3].format = VK_FORMAT_R32G32B32A32_SFLOAT;
        attributeDescriptions[3].offset = offsetof(Vertex, joinIndices);

        attributeDescriptions[4].binding = 0;
        attributeDescriptions[4].location = 4;
        attributeDescriptions[4].format = VK_FORMAT_R32G32B32A32_SFLOAT;
        attributeDescriptions[4].offset = offsetof(Vertex, weights);

        return attributeDescriptions;
    }
};
} // namespace glvm::core

// Render objects.
struct RenderPlayer {
    Vector<float, 3> position;
    Vector<float, 3> forward;
};

struct RenderActor {
    Matrix<float, 4> modelMatrix;
    std::vector<Matrix<float, 4>> jointMatrices;
    unsigned int meshID;
    unsigned int diffuseTextureIndex;
    unsigned int specularTextureIndex;
    Vector<float, 3> ambient;
    float shininess;
};

struct RenderDirectionalLight {
    Matrix<float, 4> DirectionalLightSpaceMatrix;
    Vector<float, 4> position;
    Vector<float, 4> direction;

    Vector<float, 4> ambient;
    Vector<float, 4> diffuse;
    Vector<float, 4> specular;
};

struct RenderSpotLight {
    Matrix<float, 4> SpotLigthSpaceMatrix;
    Vector<float, 3> position;
    Vector<float, 3> direction;
    float cutOff;
    float outerCutOff;

    Vector<float, 3> ambient;
    Vector<float, 3> diffuse;
    Vector<float, 3> specular;

    float constant;
    float linear;
    float quadratic;
};

#define CUBE_MAP_LAYER_NUMBER 6

struct RenderPointLight {
    Matrix<float, 4> pointLightSpaceMatrix[CUBE_MAP_LAYER_NUMBER];
    Vector<float, 3> position;

    Vector<float, 3> ambient;
    Vector<float, 3> diffuse;
    Vector<float, 3> specular;

    float constant;
    float linear;
    float quadratic;
};

struct RenderHealth {
    Vector<float, 3> position;
    float maxHealth;
    float currentHealth;
    unsigned int meshID;
};

struct RenderFont {
    Vector<float, 3> position;
    std::vector<char> font_string;
    float lifeTime;
};

struct SlotData {
    Matrix<float, 4> model;
    Vector<float, 3> color;
};

struct RenderInventory {
    std::vector<SlotData> slotData;
    unsigned int inventoryTextureID;
    unsigned int meshID;
    unsigned int row;
    unsigned int col;
};

struct RenderItem {
    Matrix<float, 4> model;
    unsigned int meshID;
    unsigned int diffuseTexureID;
};

struct RenderCrosshair {
    Matrix<float, 4> model;
    unsigned int meshID;
};
