#type vertex
#version 410

layout (location = 0) in vec2 a_Position;
layout (location = 1) in vec2 a_TexCoords;

out vec2 v_TexCoords;

void main()
{
    gl_Position = vec4(a_Position.x, a_Position.y, 0.0, 1.0); 
    v_TexCoords = a_TexCoords;
}

#type fragment
#version 410

out vec4 color;

in vec2 v_TexCoords;

uniform sampler2D u_Texture;

void main()
{
	vec4 col = texture(u_Texture, v_TexCoords);
	if (col.a == 0.0) discard;
	
	color = vec4(col.a, col.a, col.a, 1.0);
}