#version 450

// Screen-space quad: inPos is NDC, inColor.xy carries UV.
layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inColor;

layout(location = 0) out vec2 uv;

void main() {
    gl_Position = vec4(inPos, 1.0);
    uv = inColor.xy;
}