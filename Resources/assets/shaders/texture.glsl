#type vertex
#version 410
			
layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_UV;
layout(location = 2) in vec4 a_Color;
layout(location = 3) in float a_TexIndex;
layout(location = 4) in float a_TilingFactor;
layout(location = 5) in	int a_EntityID;

uniform mat4 u_ViewProjection;
uniform vec4 u_TestVec4;

out vec2 v_UV;
out vec4 v_Color;
flat out float v_TexIndex;
out float v_TilingFactor;
flat out int v_EntityID;

void main()
{
	v_UV = a_UV;
	v_Color = a_Color;
	v_TexIndex = a_TexIndex;
	v_TilingFactor = a_TilingFactor;
	v_EntityID = a_EntityID;

	gl_Position = u_ViewProjection * u_TestVec4;
}

#type fragment
#version 410
			
layout(location = 0) out vec4 color;
layout(location = 1) out int id;

in vec2 v_UV;
in vec4 v_Color;
flat in float v_TexIndex;
in float v_TilingFactor;
flat in int v_EntityID;

uniform sampler2D u_Textures[32];
uniform float u_TilingFactor;

sampler2D GetTexture()
{
	switch (v_TexIndex)
	{
		case  0: u_Textures[ 0]; break;
		case  1: u_Textures[ 1]; break;
		case  2: u_Textures[ 2]; break;
		case  3: u_Textures[ 3]; break;
		case  4: u_Textures[ 4]; break;
		case  5: u_Textures[ 5]; break;
		case  6: u_Textures[ 6]; break;
		case  7: u_Textures[ 7]; break;
		case  8: u_Textures[ 8]; break;
		case  9: u_Textures[ 9]; break;
		case 10: u_Textures[10]; break;
		case 11: u_Textures[11]; break;
		case 12: u_Textures[12]; break;
		case 13: u_Textures[13]; break;
		case 14: u_Textures[14]; break;
		case 15: u_Textures[15]; break;
		case 16: u_Textures[16]; break;
		case 17: u_Textures[17]; break;
		case 18: u_Textures[18]; break;
		case 19: u_Textures[19]; break;
		case 20: u_Textures[20]; break;
		case 21: u_Textures[21]; break;
		case 22: u_Textures[22]; break;
		case 23: u_Textures[23]; break;
		case 24: u_Textures[24]; break;
		case 25: u_Textures[25]; break;
		case 26: u_Textures[26]; break;
		case 27: u_Textures[27]; break;
		case 28: u_Textures[28]; break;
		case 29: u_Textures[29]; break;
		case 30: u_Textures[30]; break;
		case 31: u_Textures[31]; break;
	}
}

void main()
{
	sampler2D texture = GetTexture();
	color = texture(texture, v_UV * v_TilingFactor) * v_Color;
	id = v_EntityID;
}