#type vertex
#version 410

#define N_SHADOW_MAPS	4
			
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

struct ShadowMap
{
	sampler2D Sampler;
	mat4 LightMatrix;
	
	float Near;
	float Far;
};

uniform Texture2D u_NormalMap;
uniform ShadowMap u_ShadowMaps[N_SHADOW_MAPS];

uniform mat4 u_ViewProjection;
uniform mat4 u_ViewMatrix;
uniform mat4 u_NormalMatrix;
uniform mat4 u_MVP;
uniform vec3 u_CameraPosition;
uniform mat4 u_Transform;

out vec4 v_Color;
out vec3 v_Normal;
out vec2 v_TexCoords;
out vec3 v_FragPos;

out mat3 v_TangentSpace;
out vec3 v_CameraPosTangent;
out vec3 v_FragPosTangent;
out vec3 v_FragPosView;
out vec4 v_FragPosLight[N_SHADOW_MAPS];

void main()
{	
	mat3 inverseTangentSpace = mat3(1.0);
	vec3 normal = normalize(u_NormalMatrix * vec4(a_Normal, 1.0)).xyz;
	vec4 position = u_MVP * vec4(a_Position, 1.0);
	
	if (u_NormalMap.Use)
	{
		vec4 tangent = normalize(u_NormalMatrix * vec4(a_Tangent, 1.0));
		vec4 bitangent = normalize(u_NormalMatrix * vec4(a_Bitangent, 1.0));
		
		v_TangentSpace = mat3(tangent.xyz, bitangent.xyz, normal);
		inverseTangentSpace = transpose(v_TangentSpace);
	}
	else
	{
		v_Normal = normal;
		v_TangentSpace = mat3(1.0);
	}
	
	v_Color = a_Color;
	v_TexCoords = a_TexCoords0;
	v_FragPos = vec3(u_Transform * vec4(a_Position, 1.0));
	v_FragPosView = vec3(u_ViewMatrix * vec4(v_FragPos, 1.0));
	v_CameraPosTangent = inverseTangentSpace * u_CameraPosition;
	v_FragPosTangent = inverseTangentSpace * v_FragPos;
	
	for (int i=0; i<N_SHADOW_MAPS; i++)
		v_FragPosLight[i] = u_ShadowMaps[i].LightMatrix * u_Transform * vec4(a_Position, 1.0);
	
	gl_Position = position;
}

#type fragment
#version 410

#define N_MAX_LIGHTS	16
#define N_SHADOW_MAPS	4

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

struct ShadowMap
{
	sampler2D Sampler;
	mat4 LightMatrix;
	
	float Near;
	float Far;
};

in vec4 v_Color;
in vec3 v_Normal;
in vec2 v_TexCoords;
in vec3 v_FragPos;

in mat3 v_TangentSpace;
in vec3 v_CameraPosTangent;
in vec3 v_FragPosTangent;
in vec3 v_FragPosView;
in vec4 v_FragPosLight[N_SHADOW_MAPS];

layout(location = 0) out vec4 color;
layout(location = 1) out int id;

uniform int u_EntityID;
uniform vec3 u_CameraPosition;

uniform vec3 u_DirectionalLightDir;
uniform vec3 u_DirectionalLightCol;
uniform float u_DirectionalLightIntensity;

uniform float u_AmbientLightIntensity;
uniform vec3 u_AmbientLightColor;

uniform int u_NPointLights;
uniform PointLight u_PointLights[N_MAX_LIGHTS];

uniform ShadowMap u_ShadowMaps[N_SHADOW_MAPS];
uniform float u_ShadowFadeoutStart;
uniform float u_ShadowFadeoutEnd;

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

vec3 DirectionalPhong(vec3 normal, vec3 lightDir, vec3 viewDir, vec2 texCoords)
{
    // Diffuse component
    float diff = max(dot(normal, lightDir), 0.0);
	
	vec3 reflectDir = reflect(-normalize(lightDir), normal);  
	
	float specStrength;
	if (u_SpecularMap.Use)
		specStrength = texture(u_SpecularMap.Sampler, v_TexCoords * u_SpecularMap.Tiling + u_SpecularMap.Offset).r * u_SpecularMap.Intensity;
	else
		specStrength = u_SpecularStrength;
	float spec = pow(max(dot(normalize(viewDir), reflectDir), 0.0), u_SpecularShininess) * specStrength;
	
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

float GetShadows(vec3 normal, vec3 lightDir)
{
	// Get fragment depth
	float shadow = 0;
	float currDepth = abs(v_FragPosView.z);
	
	// Select shadowmap
	int layer = 3;
	for (int i=0; i<N_SHADOW_MAPS; i++)
	{
		if (currDepth < u_ShadowMaps[i].Far)
		{
			layer = i;
			break;
		}
	}
	
	// Extract data from selected map
	float near = u_ShadowMaps[layer].Near;
	float far = u_ShadowMaps[layer].Far;
	vec4 fragPosLight = v_FragPosLight[layer];
	
	vec3 projFragLight = fragPosLight.xyz / fragPosLight.w;
	
	projFragLight = projFragLight * 0.5 + 0.5;
	currDepth = projFragLight.z;
	if (currDepth  > 1.0)
		return 1.0;
	
	float cameraDistance = length(v_FragPos - u_CameraPosition);
	float attenuation = max(1.0 - ((mix(u_ShadowFadeoutStart, u_ShadowFadeoutEnd, (cameraDistance - u_ShadowFadeoutStart) / (u_ShadowFadeoutEnd - u_ShadowFadeoutStart))) - u_ShadowFadeoutStart)
		/ (u_ShadowFadeoutEnd - u_ShadowFadeoutStart), 0.0);
	float bias = max(0.002 * (1.0 - dot(normal, lightDir)), 0.001);
	
	if (projFragLight.x >= 1.0 || projFragLight.x <= 0.0 || projFragLight.y >= 1.0 || projFragLight.y <= 0.0)
		return 1.0;

	vec2 texelSize = 1.0 / textureSize(u_ShadowMaps[layer].Sampler, 0);
	for(int x = -1; x <= 1; ++x)
	{
		for(int y = -1; y <= 1; ++y)
		{
			float pcfDepth = texture(u_ShadowMaps[layer].Sampler, projFragLight.xy + vec2(x, y) * texelSize).r; 
			shadow += currDepth - bias > pcfDepth ? 1.0 : 0.0;        
		}
	}
	
	return (1.0 - (shadow / 9.0) * min(attenuation, 1.0));
}

void main()
{
	vec4 texColor;
	vec2 texCoords = v_TexCoords;
	
	// Apply parallax mapping
	if (u_HeightMap.Use)
		texCoords = ParallaxMapping();
	
	// Get texture color
	if (u_DiffuseTexture.Use)
	{
		texColor = texture(u_DiffuseTexture.Sampler, texCoords * u_DiffuseTexture.Tiling + u_DiffuseTexture.Offset);
		if (texColor.w < 0.1)
			discard;
	}
	else
		texColor = v_Color;
	
	// Compute normal vector
	vec3 normal = normalize(v_Normal);
	vec3 lightDir = normalize(u_DirectionalLightDir);	
	
	// Sample normalmap
	if (u_NormalMap.Use)
	{
		normal = normalize(v_TangentSpace * vec3(vec4(u_NormalMap.Intensity, u_NormalMap.Intensity, 1.0, 1.0) * 
			(texture(u_NormalMap.Sampler, texCoords * u_NormalMap.Tiling + u_NormalMap.Offset) * 2 - 1)));
	}
	
	// Compute shadows
	float shadow = GetShadows(normal, lightDir);
	vec3 fragToCamera = u_CameraPosition - v_FragPos;
		
	// Get directional light color
	color = texColor * vec4(u_AmbientLightColor*u_AmbientLightIntensity, 1.0) +
			texColor * vec4(shadow * DirectionalPhong(normal, lightDir, fragToCamera, texCoords), 1.0);
	
	// Get point lights color
	for (int i=0; i<u_NPointLights; i++)
		color += texColor * vec4(PointPhong(normal, u_PointLights[i], fragToCamera, u_PointLights[i].Position - v_FragPos, texCoords), 1.0);
	
	// Occlusion
	if (u_OcclusionMap.Use)
	{
		vec4 occ = texture(u_OcclusionMap.Sampler, texCoords * u_OcclusionMap.Tiling + u_OcclusionMap.Offset);
		if (occ.w > 0.0)
			color *= occ * u_OcclusionMap.Intensity;
	}
	
	// Emission map
	if (u_EmissionMap.Use)
	{
		vec3 emissiveCol = (texture(u_EmissionMap.Sampler, texCoords * u_EmissionMap.Tiling + u_EmissionMap.Offset)).xyz;
		if (length(emissiveCol) > 0)
		{
			color += vec4(emissiveCol, 1.0) * u_EmissionMap.Intensity;
			return;
		}
	}
	id = u_EntityID;
}