#version 450

// Views a shadow map depth texture. Reuses the engine's light-data descriptor
// set (binding 1 = directional shadow array, 37 = spot shadow array).
layout(set = 0, binding = 1) uniform sampler2DArray dirShadowMaps;
layout(set = 0, binding = 37) uniform sampler2DArray spotShadowMaps;

layout(push_constant) uniform PushConsts {
    // 0 = directional, 1 = spot.
    int mode;
    int layer;
    int pad0;
    int pad1;
} pc;

layout(location = 0) in vec2 uv;
layout(location = 0) out vec4 outColor;

void main() {
    float d = (pc.mode == 0)
        ? texture(dirShadowMaps, vec3(uv, float(pc.layer))).r
        : texture(spotShadowMaps, vec3(uv, float(pc.layer))).r;
    // Invert so close geometry reads bright, far reads dark.
    outColor = vec4(vec3(1.0 - clamp(d, 0.0, 1.0)), 1.0);
}
