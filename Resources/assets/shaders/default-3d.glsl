#type vertex
#version 410
			
layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Color;
layout(location = 2) in vec3 a_Normal;
layout(location = 3) in vec3 a_Tangent;
layout(location = 4) in vec3 a_Bitangent;
layout(location = 5) in vec2 a_TexCoords0;

struct Texture2D
{
	vec2 Tiling;
	vec2 Offset;
	
	float Intensity;
	bool Use;
	
	sampler2D Sampler;
};

uniform Texture2D u_NormalMap;

uniform mat4 u_ViewProjection;
uniform mat4 u_ViewMatrix;
uniform mat4 u_ProjectionMatrix;
uniform vec3 u_CameraPosition;
uniform mat4 u_Transform;

out vec4 v_Color;
out vec3 v_Normal;
out vec2 v_TexCoords;
out vec3 v_FragPos;

out mat3 v_TangentSpace;

void main()
{
	mat4 viewProj = u_ProjectionMatrix * u_ViewMatrix;
	mat4 mvp = viewProj * u_Transform;
	
	vec3 normal = mat3(transpose(inverse(u_Transform))) * a_Normal;

	v_Color = a_Color;
	if (u_NormalMap.Use)
	{
		vec4 tangent = normalize(u_Transform * vec4(a_Tangent, 1.0));
		vec4 bitangent = normalize(u_Transform * vec4(a_Bitangent, 1.0));
		
		v_TangentSpace = mat3(vec3(tangent), vec3(bitangent), normal);
	}
	// TODO: send the matrix via CPU
	else
		v_Normal = normal;
		
	v_TexCoords = a_TexCoords0;
	v_FragPos = vec3(u_Transform * vec4(a_Position, 1.0));

	gl_Position = mvp * vec4(a_Position, 1.0);
}

#type fragment
#version 410

#define N_MAX_LIGHTS	16

struct PointLight
{
	vec3 Position;
	vec3 Color;
	
	float Intensity;
	float Radius;
};

struct Texture2D
{
	vec2 Tiling;
	vec2 Offset;
	
	float Intensity;
	bool Use;
	
	sampler2D Sampler;
};

in vec4 v_Color;
in vec3 v_Normal;
in vec2 v_TexCoords;
in vec3 v_FragPos;

in mat3 v_TangentSpace;

layout(location = 0) out vec4 color;

uniform vec3 u_CameraPosition;

uniform vec3 u_DirectionalLightDir;
uniform vec3 u_DirectionalLightCol;
uniform float u_DirectionalLightIntensity;

uniform float u_AmbientLightIntensity;
uniform vec3 u_AmbientLightColor;

uniform int u_NPointLights;
uniform PointLight u_PointLights[N_MAX_LIGHTS];

uniform float u_SpecularShininess;
uniform float u_SpecularStrength;

uniform Texture2D u_DiffuseTexture;
uniform Texture2D u_NormalMap;
uniform Texture2D u_SpecularMap;
uniform Texture2D u_OcclusionMap;
uniform Texture2D u_EmissionMap;

uniform sampler2D u_RoughnessMap;
uniform sampler2D u_MetalnessMap;
uniform sampler2D u_DisplacementMap;


vec3 DirectionalPhong(vec3 normal, vec3 lightDir, vec3 viewDir)
{
    // Diffuse component
    float diff = max(dot(normal, lightDir), 0.0);
	
	vec3 reflectDir = reflect(-normalize(lightDir), normal);  
	float spec = pow(max(dot(normalize(viewDir), reflectDir), 0.0), u_SpecularShininess) * u_SpecularStrength;
	
    return (diff + spec) * u_DirectionalLightCol * u_DirectionalLightIntensity;
}

vec3 PointPhong(vec3 normal, PointLight light, vec3 viewDir, vec3 lightDir)
{
	// Attenuation
	float distance = length(lightDir);
	float attenuation = 1.0 / (1 + distance * (2 / light.Radius) + (1.0 / pow(light.Radius,2)) * pow(distance, 2));
	
	// Intensity
	float intensity = light.Intensity / pow((distance / light.Radius + 1), 2);
	
	// Diffuse component
	float diff = max(dot(normal, normalize(lightDir)), 0.0);
	
	// Specular component
	float specStrength;
	if (u_SpecularMap.Use)
		specStrength = texture(u_SpecularMap.Sampler, v_TexCoords * u_SpecularMap.Tiling + u_SpecularMap.Offset).r * u_SpecularMap.Intensity;
	else
		specStrength = u_SpecularStrength;
	vec3 reflectDir = reflect(-normalize(lightDir), normal);  
	float spec = pow(max(dot(normalize(viewDir), reflectDir), 0.0), u_SpecularShininess) * specStrength;
	
    return (diff + spec) * light.Intensity * light.Color * attenuation ;
}

void main()
{
	vec4 texColor;
	
	if (u_DiffuseTexture.Use)
	{
		texColor = texture(u_DiffuseTexture.Sampler, v_TexCoords * u_DiffuseTexture.Tiling + u_DiffuseTexture.Offset);
		if (texColor.w < 0.1)
			discard;
	}
	else
	{
		texColor = v_Color;
	}
	
	vec3 normal = normalize(v_Normal);
	vec3 lightDir = normalize(u_DirectionalLightDir);	
	
	// Sample normalmap
	if (u_NormalMap.Use)
	{
		normal = normalize(v_TangentSpace * vec3(vec4(u_NormalMap.Intensity, u_NormalMap.Intensity, 1.0, 1.0) * 
			(texture(u_NormalMap.Sampler, v_TexCoords * u_NormalMap.Tiling + u_NormalMap.Offset) * 2 - 1)));
	}
		
	// Get color
	color = texColor * vec4(u_AmbientLightColor*u_AmbientLightIntensity, 1.0) + 
			texColor * vec4(DirectionalPhong(normal, lightDir, u_CameraPosition - v_FragPos), 1.0);
	
	for (int i=0; i<u_NPointLights; i++)
		color += texColor * vec4(PointPhong
			(normal, u_PointLights[i], u_CameraPosition - v_FragPos, 
			u_PointLights[i].Position - v_FragPos), 1.0);
	
	// Occlusion
	if (u_OcclusionMap.Use)
	{
		vec4 occ = texture(u_OcclusionMap.Sampler, v_TexCoords * u_OcclusionMap.Tiling + u_OcclusionMap.Offset);
		if (occ.w > 0.0)
			color *= occ * u_OcclusionMap.Intensity;
	}
	
	if (u_EmissionMap.Use)
	{
		vec3 emissiveCol = (texture(u_EmissionMap.Sampler, v_TexCoords * u_EmissionMap.Tiling + u_EmissionMap.Offset)).xyz;
		if (length(emissiveCol) > 0)
		{
			color += vec4(emissiveCol, 1.0) * u_EmissionMap.Intensity;
			return;
		}
	}
}