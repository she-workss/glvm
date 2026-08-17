#version 450

layout(location = 0) in vec2 inFragmentTextureCoordinate;

layout(set = 1, binding = 0) uniform sampler2D fontAtlas;

layout(location = 0) out vec4 outColor;

void main()
{
	vec4 color = vec4(texture(fontAtlas, inFragmentTextureCoordinate));

	if ( color.a == 1 ) {
		discard;
	}
	
	outColor = color;
}
