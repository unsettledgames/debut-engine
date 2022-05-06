#type vertex
#version 410
			
layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_UV;
layout(location = 2) in vec4 a_Color;
layout(location = 3) in float a_TexIndex;
layout(location = 4) in float a_TilingFactor;
layout(location = 5) in	int a_EntityID;

uniform mat4 u_ViewProjection;

out vec2 v_UV;
out vec4 v_Color;
out float v_TexIndex;
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
in float v_TexIndex;
in float v_TilingFactor;
flat in int v_EntityID;

uniform sampler2D u_Textures[32];
uniform float u_TilingFactor;

void main()
{
	color = texture(u_Textures[int(v_TexIndex)], v_UV * v_TilingFactor) * v_Color;
	id = v_EntityID;
}