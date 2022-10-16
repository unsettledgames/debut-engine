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

	gl_Position = u_ViewProjection * vec4(a_Position, 1.0);
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

vec4 GetTexColor()
{
	switch (int(v_TexIndex))
	{
		case  0: return texture(u_Textures[ 0], v_UV * v_TilingFactor); break;
		case  1: return texture(u_Textures[ 1], v_UV * v_TilingFactor); break;
		case  2: return texture(u_Textures[ 2], v_UV * v_TilingFactor); break;
		case  3: return texture(u_Textures[ 3], v_UV * v_TilingFactor); break;
		case  4: return texture(u_Textures[ 4], v_UV * v_TilingFactor); break;
		case  5: return texture(u_Textures[ 5], v_UV * v_TilingFactor); break;
		case  6: return texture(u_Textures[ 6], v_UV * v_TilingFactor); break;
		case  7: return texture(u_Textures[ 7], v_UV * v_TilingFactor); break;
		case  8: return texture(u_Textures[ 8], v_UV * v_TilingFactor); break;
		case  9: return texture(u_Textures[ 9], v_UV * v_TilingFactor); break;
		case 10: return texture(u_Textures[10], v_UV * v_TilingFactor); break;
		case 11: return texture(u_Textures[11], v_UV * v_TilingFactor); break;
		case 12: return texture(u_Textures[12], v_UV * v_TilingFactor); break;
		case 13: return texture(u_Textures[13], v_UV * v_TilingFactor); break;
		case 14: return texture(u_Textures[14], v_UV * v_TilingFactor); break;
		case 15: return texture(u_Textures[15], v_UV * v_TilingFactor); break;
		case 16: return texture(u_Textures[16], v_UV * v_TilingFactor); break;
		case 17: return texture(u_Textures[17], v_UV * v_TilingFactor); break;
		case 18: return texture(u_Textures[18], v_UV * v_TilingFactor); break;
		case 19: return texture(u_Textures[19], v_UV * v_TilingFactor); break;
		case 20: return texture(u_Textures[20], v_UV * v_TilingFactor); break;
		case 21: return texture(u_Textures[21], v_UV * v_TilingFactor); break;
		case 22: return texture(u_Textures[22], v_UV * v_TilingFactor); break;
		case 23: return texture(u_Textures[23], v_UV * v_TilingFactor); break;
		case 24: return texture(u_Textures[24], v_UV * v_TilingFactor); break;
		case 25: return texture(u_Textures[25], v_UV * v_TilingFactor); break;
		case 26: return texture(u_Textures[26], v_UV * v_TilingFactor); break;
		case 27: return texture(u_Textures[27], v_UV * v_TilingFactor); break;
		case 28: return texture(u_Textures[28], v_UV * v_TilingFactor); break;
		case 29: return texture(u_Textures[29], v_UV * v_TilingFactor); break;
		case 30: return texture(u_Textures[30], v_UV * v_TilingFactor); break;
		case 31: return texture(u_Textures[31], v_UV * v_TilingFactor); break;
	}
	
	return texture(u_Textures[0], v_UV * v_TilingFactor);
}

void main()
{
	vec4 texColor = GetTexColor();
	color = texColor * v_Color;
	id = v_EntityID;
}