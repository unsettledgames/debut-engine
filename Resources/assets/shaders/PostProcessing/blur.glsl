#type vertex
#version 410

layout (location = 0) in vec2 a_Position;
layout(location = 1) in vec2 a_TexCoords;

out vec2 v_TexCoords;

void main()
{
	v_TexCoords = a_TexCoords;
	gl_Position = vec4(a_Position, 0.0, 1.0);
}

#type fragment
#version 410

#define MAX_KERNEL_SIZE	256

uniform sampler2D u_Texture;

uniform int u_Size;
uniform float u_Sharpness;
uniform int u_Vertical;
uniform float u_Kernel[MAX_KERNEL_SIZE];

in vec2 v_TexCoords;

out vec4 color;

void main()
{
	vec2 texSize = textureSize(u_Texture, 0);
	vec2 texelSize = vec2(1.0 / texSize.x, 1.0 / texSize.y);
	vec4 col = vec4(0.0);
	float sum = 0;
	
	for (int i=-u_Size; i<=u_Size; i++)
	{
		vec2 texCoords = v_TexCoords + vec2((1 - u_Vertical) * texelSize.x, u_Vertical * texelSize.y) * i;
		col += texture(u_Texture, texCoords) * u_Kernel[i + u_Size];
		sum += u_Kernel[i + u_Size];
	}
	
	color = vec4(col.xyz, 1.0);
}