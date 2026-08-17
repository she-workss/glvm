#version 450

layout(location = 0) in vec2 inFragmentTextureCoordinate;

layout(location = 0) out vec4 outColor;

void main()
{
	vec4 color = vec4(0.0, 0.5, 0.0, 1.0);
	
	outColor = color;
}
