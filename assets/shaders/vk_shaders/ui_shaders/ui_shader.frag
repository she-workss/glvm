#version 450

layout(location = 0) in vec2 inFragmentTextureCoordinate;
layout(location = 1) in vec3 inColor;

layout(set = 1, binding = 0) uniform sampler2D inventoryTexture;

layout(location = 0) out vec4 outColor;

void main()
{
	vec4 color = vec4(texture(inventoryTexture, inFragmentTextureCoordinate));
	
	outColor = vec4(color.r + inColor.x, color.g + inColor.y, color.b + inColor.z, color.a);
}
