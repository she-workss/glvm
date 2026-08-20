#version 450

#define MAX_JOINTS_NUMBER 128

layout(set = 0, binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 lightSpace;
    mat4 jointMatrices[MAX_JOINTS_NUMBER];
}

ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTextureCoordinate;
layout(location = 3) in vec4 inJointIndices;
layout(location = 4) in vec4 inWeights;

void main() {
    mat4 skinMatrix;
    if (int(inJointIndices.x) != -1) {
        skinMatrix = inWeights.x * ubo.jointMatrices[int(inJointIndices.x)]
            + inWeights.y * ubo.jointMatrices[int(inJointIndices.y)]
            + inWeights.z * ubo.jointMatrices[int(inJointIndices.z)]
            + inWeights.w * ubo.jointMatrices[int(inJointIndices.w)];
    } else {
        skinMatrix = mat4(
            1.0,
            0.0,
            0.0,
            0.0,
            0.0,
            1.0,
            0.0,
            0.0,
            0.0,
            0.0,
            1.0,
            0.0,
            0.0,
            0.0,
            0.0,
            1.0
        );
    }
    vec4 worldPosition = ubo.model * skinMatrix * vec4(inPosition, 1.0);
    gl_Position = ubo.lightSpace * worldPosition;
}
