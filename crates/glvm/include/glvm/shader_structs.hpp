#pragma once

#include "glvm/vertex_math.hpp"

#include <stdint.h>

namespace glvm::core {
#define SHADOW_MAP_SIZE 1024
#define FLAT_SHADOW_MAP_SIZE 2048

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
    alignas(16) Matrix<float, 4> spotSpaceMatrix[SPOT_LIGHTS_NUMBER];
    alignas(16) uint32_t spotLightsNumber;

    alignas(16) Matrix<float, 4> dirSpaceMatrix[DIRECTIONAL_LIGHTS_NUMBER];
    alignas(16) uint32_t directionalLightsNumber;
};

struct alignas(64) ModelMatrixUBO {
    Matrix<float, 4> model;
    Matrix<float, 4> view;
    Matrix<float, 4> proj;
    Matrix<float, 4> jointMatrices[MAX_JOINTS_NUMBER];

    Vector<float, 3> ambient;
    float shininess;

    alignas(16) Matrix<float, 4> spotSpaceMatrix[SPOT_LIGHTS_NUMBER];
    alignas(16) uint32_t spotLightsNumber;

    alignas(16) Matrix<float, 4> dirSpaceMatrix[DIRECTIONAL_LIGHTS_NUMBER];
    alignas(16) uint32_t directionalLightsNumber;
};

struct alignas(16) ShadowMapMatrixUBO {
    Matrix<float, 4> model;
    Matrix<float, 4> lightSpaceMatrix;
    Matrix<float, 4> jointMatrices[MAX_JOINTS_NUMBER];
};

struct alignas(16) SpotLightShadowMapMatrixUBO {
    Matrix<float, 4> model;
    Matrix<float, 4> lightSpaceMatrix;
};

struct alignas(64) PointLightShadowMapMatrixUBO {
    Matrix<float, 4> model;
    Matrix<float, 4> lightSpaceMatrix;
    Vector<float, 3> lightPosition;
    float farPlane;
    Matrix<float, 4> jointMatrices[MAX_JOINTS_NUMBER];
};

struct alignas(16) UniformBufferObjectLightUBO {
    Vector<float, 3> lightPosition;
    float farPlane;
};

struct alignas(16) DirectionalLight {
    Vector<float, 4> position;
    Vector<float, 4> direction;

    Vector<float, 4> ambient;
    Vector<float, 4> diffuse;
    Vector<float, 4> specular;
};

struct alignas(16) PointLight {
    Vector<float, 3> position;
    float padding0;

    Vector<float, 3> ambient;
    float padding1;
    Vector<float, 3> diffuse;
    float padding2;

    Vector<float, 3> specular;
    float constant;
    float linear;
    float quadratic;
};

struct alignas(16) SpotLight {
    alignas(16) Vector<float, 3> position;
    alignas(16) Vector<float, 3> direction;
    float cutOff;
    float outerCutOff;

    alignas(16) Vector<float, 3> ambient;
    alignas(16) Vector<float, 3> diffuse;
    alignas(16) Vector<float, 3> specular;

    float constant;
    float linear;
    float quadratic;
};

struct alignas(64) LightData {
    Vector<float, 2> tilesetTilesCount;
    int tilesRaw;
    int tilesColumn;

    alignas(16) Vector<float, 3> viewPosition;

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

    // Debug: 0 = off, 1 = directional, 2 = spot. When set, the main shader
    // renders the shadow map depth projected onto the scene instead of
    // lighting (visualized from the normal moving camera).
    int debugShadowMode;
    int debugShadowLight;
    int shadowsEnabled;
};

struct alignas(64) HUD_UBO {
    Matrix<float, 4> view;
    Matrix<float, 4> proj;
    Vector<float, 3> entityPosition;
    int isHudExists;
    float maxHP;
    float currentHP;
    float highestY;
};

struct alignas(64) HUD_SCREEN_UBO {
    Matrix<float, 4> model;
};

struct alignas(64) FONT_UBO {
    Matrix<float, 4> view;
    Matrix<float, 4> proj;
    Vector<float, 3> position;
    float scale;
};

struct alignas(64) UI_UBO {
    Matrix<float, 4> model;
    Vector<float, 3> color;
};

struct alignas(64) VIRTUAL_TEXTURE_UBO {};

struct alignas(64) SDF_UBO {
    Matrix<float, 4> model;
    float iTime;
};

} // namespace glvm::core
