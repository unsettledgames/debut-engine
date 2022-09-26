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
out vec3 v_CameraPosTangent;
out vec3 v_FragPosTangent;

void main()
{
	mat4 viewProj = u_ProjectionMatrix * u_ViewMatrix;
	mat4 mvp = viewProj * u_Transform;
	
	// TODO: send the matrix via CPU
	mat4 normalMatrix = transpose(inverse(u_Transform));
	vec3 normal = normalize(normalMatrix * vec4(a_Normal, 1.0)).xyz;
	vec4 position = mvp * vec4(a_Position, 1.0);

	v_Color = a_Color;
	if (u_NormalMap.Use)
	{
		vec4 tangent = normalize(normalMatrix * vec4(a_Tangent, 1.0));
		vec4 bitangent = normalize(normalMatrix * vec4(a_Bitangent, 1.0));
		
		v_TangentSpace = mat3(tangent.xyz, bitangent.xyz, normal);
	}
	else
	{
		v_Normal = normal;
		v_TangentSpace = mat3(1.0);
	}
	
	mat3 inverseTangentSpace = transpose(v_TangentSpace);
	
	v_TexCoords = a_TexCoords0;
	v_FragPos = vec3(u_Transform * vec4(a_Position, 1.0));
	v_CameraPosTangent = inverseTangentSpace * u_CameraPosition;
	v_FragPosTangent = inverseTangentSpace * v_FragPos;
	
	gl_Position = position;
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
in vec3 v_CameraPosTangent;
in vec3 v_FragPosTangent;

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
uniform Texture2D u_HeightMap;

uniform sampler2D u_RoughnessMap;
uniform sampler2D u_MetalnessMap;
uniform sampler2D u_DisplacementMap;


// TODO: use specular maps
vec3 DirectionalPhong(vec3 normal, vec3 lightDir, vec3 viewDir, vec2 texCoords)
{
    // Diffuse component
    float diff = max(dot(normal, lightDir), 0.0);
	
	vec3 reflectDir = reflect(-normalize(lightDir), normal);  
	float spec = pow(max(dot(normalize(viewDir), reflectDir), 0.0), u_SpecularShininess) * u_SpecularStrength;
	
    return (diff + spec) * u_DirectionalLightCol * u_DirectionalLightIntensity;
}

vec3 PointPhong(vec3 normal, PointLight light, vec3 viewDir, vec3 lightDir, vec2 texCoords)
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
	
    return (diff + spec) * light.Intensity * light.Color * attenuation;
}

vec2 ParallaxMapping()
{
	const float minLayers = 10.0;
	const float maxLayers = 32.0;
	
	vec3 viewDir = normalize(v_CameraPosTangent - v_FragPosTangent);
	float nLayers = mix(maxLayers, minLayers, max(dot(vec3(0.0, 0.0, 1.0), viewDir), 0.0));
	
	float layerDepth = 1.0 / nLayers;
	float currDepth = 0.0;
	
	vec2 offset = (viewDir.xy / viewDir.z) * u_HeightMap.Intensity;
	vec2 delta = offset / nLayers;
	
	vec2 currTexCoords = v_TexCoords;
	float sampleHeight = 1 - texture(u_HeightMap.Sampler, currTexCoords).x;
	
	while (currDepth  < sampleHeight)
	{
		currTexCoords -= delta;
		sampleHeight = 1 - texture(u_HeightMap.Sampler, currTexCoords).x;
		currDepth += layerDepth;
	}
	
	vec2 prevTexCoords = currTexCoords + delta;
	float depthAfter = sampleHeight - currDepth;
	float depthBefore = (1 - texture(u_HeightMap.Sampler, prevTexCoords).x) - currDepth + layerDepth;
	// Interpolate
	float weight = depthAfter / (depthAfter - depthBefore);
	
	return prevTexCoords * weight + currTexCoords * (1.0 - weight);
}

void main()
{
	vec4 texColor;
	vec2 texCoords = v_TexCoords;
	
	if (u_HeightMap.Use)
	{
		texCoords = ParallaxMapping();
	}
	
	if (u_DiffuseTexture.Use)
	{
		texColor = texture(u_DiffuseTexture.Sampler, texCoords * u_DiffuseTexture.Tiling + u_DiffuseTexture.Offset);
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
			(texture(u_NormalMap.Sampler, texCoords * u_NormalMap.Tiling + u_NormalMap.Offset) * 2 - 1)));
	}
		
	// Get color
	color = texColor * vec4(u_AmbientLightColor*u_AmbientLightIntensity, 1.0) + 
			texColor * vec4(DirectionalPhong(normal, lightDir, u_CameraPosition - v_FragPos, texCoords), 1.0);
	
	for (int i=0; i<u_NPointLights; i++)
		color += texColor * vec4(PointPhong
			(normal, u_PointLights[i], u_CameraPosition - v_FragPos, 
			u_PointLights[i].Position - v_FragPos, texCoords), 1.0);
	
	// Occlusion
	if (u_OcclusionMap.Use)
	{
		vec4 occ = texture(u_OcclusionMap.Sampler, texCoords * u_OcclusionMap.Tiling + u_OcclusionMap.Offset);
		if (occ.w > 0.0)
			color *= occ * u_OcclusionMap.Intensity;
	}
	
	if (u_EmissionMap.Use)
	{
		vec3 emissiveCol = (texture(u_EmissionMap.Sampler, texCoords * u_EmissionMap.Tiling + u_EmissionMap.Offset)).xyz;
		if (length(emissiveCol) > 0)
		{
			color += vec4(emissiveCol, 1.0) * u_EmissionMap.Intensity;
			return;
		}
	}
}