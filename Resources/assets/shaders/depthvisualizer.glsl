#type vertex
#version 410
			
layout(location = 0) in vec3 a_Position;
layout(location = 6) in int a_EntityID;

uniform mat4 u_ViewProjection;
uniform mat4 u_ViewMatrix;
uniform mat4 u_Transform;
uniform mat4 u_ProjectionMatrix;

flat out int v_EntityID;

void main()
{
	mat4 viewProj = u_ProjectionMatrix * u_ViewMatrix;
	mat4 mvp = viewProj * u_Transform;
	
	vec4 position = mvp * vec4(a_Position, 1.0);

	v_EntityID = a_EntityID;
	gl_Position = position;
}

#type fragment
#version 410

flat in int v_EntityID;

layout(location = 0) out vec4 color;
layout(location = 1) out int id;
uniform float u_NearPlane;
uniform float u_FarPlane;

void main()
{
	id = v_EntityID;
	
	float depth = gl_FragCoord.z * 2.0 - 1.0;
	depth = (2.0 * u_NearPlane * u_FarPlane) / (u_FarPlane + u_NearPlane - depth * (u_FarPlane - u_NearPlane));	
	color = vec4(vec3(depth / u_FarPlane), 1.0);
}