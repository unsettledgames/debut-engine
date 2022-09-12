#type vertex
#version 410
			
layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Color;
layout(location = 2) in vec3 a_Normal;
layout(location = 5) in vec2 a_TexCoords0;

uniform mat4 u_ViewProjection;
uniform mat4 u_Transform;

out vec4 v_Color;
out vec3 v_Normal;
out vec2 v_TexCoords;

void main()
{
	v_Color = a_Color;
	v_Normal = mat3(u_Transform) * a_Normal;
	v_TexCoords = a_TexCoords0;

	gl_Position = u_ViewProjection * u_Transform * vec4(a_Position, 1.0);
}

#type fragment
#version 410

in vec4 v_Color;
in vec3 v_Normal;
in vec2 v_TexCoords;

layout(location = 0) out vec4 color;

uniform vec3 u_DirectionalLightDir;
uniform vec3 u_DirectionalLightCol;
uniform float u_DirectionalLightIntensity;

uniform float u_AmbientLightIntensity;
uniform vec3 u_AmbientLightColor;

uniform sampler2D u_DiffuseTexture;
uniform sampler2D u_NormalMap;
uniform sampler2D u_RoughnessMap;
uniform sampler2D u_MetalnessMap;
uniform sampler2D u_DisplacementMap;

vec3 phong(vec3 normal, vec3 lightDir)
{
    // Diffuse component
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * vec3(1.0, 1.0, 1.0);
	
    return u_DirectionalLightCol * diffuse * u_DirectionalLightIntensity;
}

void main()
{
	vec4 texColor = texture(u_DiffuseTexture, v_TexCoords);
	if (texColor.w < 0.1)
		discard;
		
	color = texColor * vec4(u_AmbientLightColor*u_AmbientLightIntensity, 1.0) + 
			texColor * vec4(phong(normalize(v_Normal), normalize(u_DirectionalLightDir)), 1.0);
}