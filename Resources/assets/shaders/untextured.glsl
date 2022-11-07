#type vertex
#version 410
			
layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Color;
layout(location = 2) in vec3 a_Normal;
layout(location = 3) in vec3 a_Tangent;
layout(location = 4) in vec3 a_Bitangent;
layout(location = 6) in int a_EntityID;

uniform mat4 u_ViewProjection;
uniform mat4 u_ViewMatrix;
uniform mat4 u_ProjectionMatrix;
uniform vec3 u_CameraPosition;
uniform mat4 u_Transform;

out vec4 v_Color;
out vec3 v_Normal;
out vec3 v_FragPos;

out mat3 v_TangentSpace;
out vec3 v_CameraPosTangent;
out vec3 v_FragPosTangent;

flat out int v_EntityID;

void main()
{
	mat4 viewProj = u_ProjectionMatrix * u_ViewMatrix;
	mat4 mvp = viewProj * u_Transform;
	
	// TODO: send the matrix via CPU
	mat4 normalMatrix = transpose(inverse(u_Transform));
	vec3 normal = normalize(normalMatrix * vec4(a_Normal, 1.0)).xyz;
	vec4 position = mvp * vec4(a_Position, 1.0);

	v_Color = a_Color;

	v_Normal = normal;
	v_TangentSpace = mat3(1.0);
	
	mat3 inverseTangentSpace = transpose(v_TangentSpace);
	
	v_FragPos = vec3(u_Transform * vec4(a_Position, 1.0));
	v_CameraPosTangent = inverseTangentSpace * u_CameraPosition;
	v_FragPosTangent = inverseTangentSpace * v_FragPos;
	v_EntityID = a_EntityID;
	
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

in vec4 v_Color;
in vec3 v_Normal;
in vec3 v_FragPos;
flat in int v_EntityID;

in mat3 v_TangentSpace;
in vec3 v_CameraPosTangent;
in vec3 v_FragPosTangent;

layout(location = 0) out vec4 color;
layout(location = 1) out int id;

uniform vec3 u_CameraPosition;

uniform vec3 u_DirectionalLightDir;
uniform vec3 u_DirectionalLightCol;
uniform float u_DirectionalLightIntensity;

uniform float u_AmbientLightIntensity;
uniform vec3 u_AmbientLightColor;

uniform int u_NPointLights;
uniform PointLight u_PointLights[N_MAX_LIGHTS];


vec3 DirectionalPhong(vec3 normal, vec3 lightDir, vec3 viewDir)
{
    // Diffuse component
    float diff = max(dot(normal, lightDir), 0.0);
	
	vec3 reflectDir = reflect(-normalize(lightDir), normal);  
	
	float specStrength = 1.0;
	float spec = pow(max(dot(normalize(viewDir), reflectDir), 0.0), 4) * specStrength;
	
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
	float specStrength = 1.0;
	vec3 reflectDir = reflect(-normalize(lightDir), normal);  
	float spec = pow(max(dot(normalize(viewDir), reflectDir), 0.0), 4.0) * specStrength;
	
    return (diff + spec) * light.Intensity * light.Color * attenuation;
}

void main()
{
	vec4 texColor = vec4(0.7f);
	
	vec3 normal = normalize(v_Normal);
	vec3 lightDir = normalize(u_DirectionalLightDir);	
		
	// Get color
	color = texColor * vec4(u_AmbientLightColor*u_AmbientLightIntensity, 1.0) + 
			texColor * vec4(DirectionalPhong(normal, lightDir, u_CameraPosition - v_FragPos), 1.0);
	
	for (int i=0; i<u_NPointLights; i++)
		color += texColor * vec4(PointPhong
			(normal, u_PointLights[i], u_CameraPosition - v_FragPos, 
			u_PointLights[i].Position - v_FragPos), 1.0);
	
	id = v_EntityID;
}