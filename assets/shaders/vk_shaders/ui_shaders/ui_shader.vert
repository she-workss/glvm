#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTextureCoordinate;
layout(location = 3) in vec4 inJointIndices;
layout(location = 4) in vec4 inWeights;

layout(location = 0) out vec2 outFragmentTextureCoordinate;
layout(location = 1) out vec3 outColor;

layout(set = 0, binding = 0) uniform UI_UBO {
	mat4 model;
	vec3 color;
} ui_ubo;

void main()
{
	mat4 scaleMatrix = mat4(
		1.0, 0.0, 0.0, 0.0,
		0.0, 1.777, 0.0, 0.0,
		0.0, 0.0, 1.0, 0.0,
		0.0, 0.0, 0.0, 1.0
		);
	
	outFragmentTextureCoordinate = inTextureCoordinate;
	outColor = ui_ubo.color;
	gl_Position = ui_ubo.model * scaleMatrix * vec4(inPosition, 1.0);
}
