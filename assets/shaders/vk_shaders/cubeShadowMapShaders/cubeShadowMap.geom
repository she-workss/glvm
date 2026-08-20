#version 450

layout(triangles) in;
layout(triangle_strip, max_vertices = 18) out;

layout(location = 3) uniform UniformBufferLightSpace {
    mat4 shadowMatrices[6];
};

// FragmentPosition from GS (output per emitvertex).
layout(location = 4) out vec4 fragmentPosition;

void main() {
    for (int face = 0; face < 6; ++face) {
        // Built-in variable that specifies to which face we render.
        gl_Layer = face;
        for (int i = 0; i < 3; ++i) {
            fragmentPosition = gl_in[i].gl_Position;
            gl_Position = shadowMatrices[face] * fragmentPosition;
            EmitVertex();
        }
        EndPrimitive();
    }
}
