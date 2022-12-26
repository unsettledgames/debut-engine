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

uniform sampler2D u_Texture;
uniform float u_Intensity;

in vec2 v_TexCoords;

void main()
{
	vec4 col = texture(u_Texture, v_TexCoords);
	float rightIntensity = 1.0 - u_Intensity;
	
	col.g *= rightIntensity; col.b *= rightIntensity;
	gl_FragColor = col;
}