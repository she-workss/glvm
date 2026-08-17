#ifndef SHADER_STRUCTS
#define SHADER_STRUCTS

#include "glvm/VertexMath.hpp"

#include <stdint.h>

namespace GLVM::core {
#define SHADOW_MAP_SIZE 640

#define VK_DEBUG_IMAGE_SET_RED "\x1b[31mVULKAN DEBUG IMAGE\x1b[0m"
#define VK_DEBUG_DESCRIPTOR_SET_RED "\x1b[31mVULKAN DEBUG DESCRIPTOR SET\x1b[0m"
#define VK_DEBUG_DESCRIPTOR_SET_LAYOUT_RED                                     \
    "\x1b[31mVULKAN DEBUG DESCRIPTOR SET LAYOUT\x1b[0m"
#define VK_DEBUG_PIPELINE_RED "\x1b[31mVULKAN DEBUG PIPELINE:\x1b[0m"
#define VK_DEBUG_PIPELINE_LAYOUT_RED                                           \
    "\x1b[31mVULKAN DEBUG PIPELINE LAYOUT:\x1b[0m"

#define DIRECTIONAL_LIGHTS_NUMBER 4
#define POINT_LIGHTS_NUMBER 32
#define SPOT_LIGHTS_NUMBER 8

#define MAX_JOINTS_NUMBER 128

#define INDIRECT_TEXTURE_WIDTH 7
#define INDIRECT_TEXTURE_HEIGHT 5
#define TILESET_ROW 8
#define TILESET_COLUMN 8

struct LightSpaceMatrixUBO {
    alignas(16) mat4 spotSpaceMatrix[SPOT_LIGHTS_NUMBER];
    alignas(16) uint32_t spotLightsNumber;

    alignas(16) mat4 dirSpaceMatrix[DIRECTIONAL_LIGHTS_NUMBER];
    alignas(16) uint32_t directionalLightsNumber;
};

struct alignas(64) ModelMatrixUBO {
    mat4 model;
    mat4 view;
    mat4 proj;
    mat4 jointMatrices[MAX_JOINTS_NUMBER];

    vec3 ambient;
    float shininess;

    alignas(16) mat4 spotSpaceMatrix[SPOT_LIGHTS_NUMBER];
    alignas(16) uint32_t spotLightsNumber;

    alignas(16) mat4 dirSpaceMatrix[DIRECTIONAL_LIGHTS_NUMBER];
    alignas(16) uint32_t directionalLightsNumber;
};

struct alignas(16) ShadowMapMatrixUBO {
    mat4 model;
    mat4 lightSpaceMatrix;
    mat4 jointMatrices[MAX_JOINTS_NUMBER];
};

struct alignas(16) SpotLightShadowMapMatrixUBO {
    mat4 model;
    mat4 lightSpaceMatrix;
};

struct alignas(64) PointLightShadowMapMatrixUBO {
    mat4 model;
    mat4 lightSpaceMatrix;
    vec3 lightPosition;
    float farPlane;
    mat4 jointMatrices[MAX_JOINTS_NUMBER];
};

struct alignas(16) UniformBufferObjectLightUBO {
    vec3 lightPosition;
    float farPlane;
};

struct alignas(16) DirectionalLight {
    vec4 position;
    vec4 direction;

    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
};

struct alignas(16) PointLight {
    vec3 position;
    float padding0;

    vec3 ambient;
    float padding1;
    vec3 diffuse;
    float padding2;

    vec3 specular;
    float constant;
    float linear;
    float quadratic;
};

struct alignas(16) SpotLight {
    alignas(16) vec3 position;
    alignas(16) vec3 direction;
    float cutOff;
    float outerCutOff;

    alignas(16) vec3 ambient;
    alignas(16) vec3 diffuse;
    alignas(16) vec3 specular;

    float constant;
    float linear;
    float quadratic;
};

struct alignas(64) LightData {
    vec2 tilesetTilesCount;
    int tilesRaw;
    int tilesColumn;

    alignas(16) vec3 viewPosition;

    PointLight pointLights[POINT_LIGHTS_NUMBER];
    int pointLightsArraySize;
    float farPlane;
    int padding0;
    int padding1;

    DirectionalLight directionalLights[DIRECTIONAL_LIGHTS_NUMBER];
    alignas(16) int directionalLightsArraySize;

    SpotLight spotLights[SPOT_LIGHTS_NUMBER];
    int spotLightArraySize;
    int padding2;
    int padding3;
    int padding4;

    Vector<int, 4>
        indirectTexture[INDIRECT_TEXTURE_WIDTH * INDIRECT_TEXTURE_HEIGHT / 4 + 1];
};

struct alignas(64) HUD_UBO {
    mat4 view;
    mat4 proj;
    vec3 entityPosition;
    int isHudExists;
    float maxHP;
    float currentHP;
    float highestY;
};

struct alignas(64) HUD_SCREEN_UBO {
    mat4 model;
};

struct alignas(64) FONT_UBO {
    mat4 view;
    mat4 proj;
    vec3 position;
    float scale;
};

struct alignas(64) UI_UBO {
    mat4 model;
    vec3 color;
};

struct alignas(64) VIRTUAL_TEXTURE_UBO {};

struct alignas(64) SDF_UBO {
    mat4 model;
    float iTime;
};

} // namespace GLVM::core

#endif
