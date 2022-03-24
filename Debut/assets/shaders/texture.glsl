#type vertex
#version 410
			
layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_UV;

uniform mat4 u_ViewProjection;
uniform mat4 u_Transform;

out vec2 v_UV;

void main()
{
	v_UV = a_UV;

	gl_Position = u_ViewProjection * u_Transform * vec4(a_Position, 1.0);
}

#type fragment
#version 410
			
layout(location = 0) out vec4 color;
layout(location = 1) in vec2 v_UV;

uniform sampler2D u_Texture;
uniform vec4 u_Color;

void main()
{
	color = texture(u_Texture, v_UV * 8) * u_Color;
}