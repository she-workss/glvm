#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTextureCoordinate;
layout(location = 3) in vec4 inJointIndices;
layout(location = 4) in vec4 inWeights;

layout(location = 0) out vec2 outFragmentTextureCoordinate;

layout(set = 0, binding = 0) uniform FONT_UBO {
  mat4 model;
  float iTime;
} matrix_ubo;

//layout(location = 0) out vec2 uv;

vec2 positions[3] = vec2[](
    vec2(-1.0, -1.0),
    vec2( 3.0, -1.0),
    vec2(-1.0,  3.0)
);

layout(location = 1) out float iTime;

void main()
{
    vec2 pos = positions[gl_VertexIndex];
	iTime = matrix_ubo.iTime;

	mat4 scaleMatrix = mat4(
		1.0, 0.0, 0.0, 0.0,
		0.0, -1.0, 0.0, 0.0,
		0.0, 0.0, 1.0, 0.0,
		0.0, 0.0, 0.0, 1.0
		);

	//    uv = pos * 0.5 + 0.5;
    gl_Position = vec4(pos, 0.0, 1.0);
}
