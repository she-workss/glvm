#pragma once

#include <cstdint>
#include <vector>

namespace glvm::ecs {
struct TextureHandle {
    uint32_t id;
};

struct Texture {
    // This field using to choose specific instance of texture image in Vulkan.
    unsigned int vkAvailableInnerId_ = 0;
    unsigned int vkInnerIdLimit_ = 10;

    const char* path_to_image = "";
    std::vector<unsigned int> entitiesOwnsThisTypeOfTexture_ = {};
    unsigned int id_ = 0;
    unsigned int iWidth_ = 0;
    unsigned int iHeight_ = 0;
    unsigned int dat_length_ = 0;
    unsigned char* u_iData_ = 0;
};
} // namespace glvm::ecs
