#version 450

layout(location = 0) in VS_OUT {
	int value;
	vec4 vertexPosition;
	float maxHP;
	float currentHP;
	float highestY;
} fs_in;

layout(location = 0) out vec4 outColor;

void main()
{
	float hpScale = (fs_in.currentHP / fs_in.maxHP) * 2.0;
	
//	vec4 color = vec4((fs_in.currentHP / fs_in.maxHP) / fs_in.maxHP, fs_in.maxHP - fs_in.currentHP, 0.0, 1.0);
	vec4 color = vec4(0.0, 0.0, 0.0, 1.0);
	
	if ( fs_in.vertexPosition.y < fs_in.highestY - 1.0 + hpScale )
		color = vec4(0.1, 0.5, 0.1, 1.0);
	else
		color = vec4(0.5, 0.1, 0.1, 1.0);
	
	outColor = color;
}
