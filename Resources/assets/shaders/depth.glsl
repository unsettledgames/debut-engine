#type vertex
#version 410
			
layout(location = 0) in vec3 a_Position;

uniform mat4 u_ViewProjection;
uniform mat4 u_ViewMatrix;
uniform mat4 u_Transform;
uniform mat4 u_ProjectionMatrix;

void main()
{
	mat4 viewProj = u_ProjectionMatrix * u_ViewMatrix;
	mat4 mvp = viewProj * u_Transform;
	
	vec4 position = mvp * vec4(a_Position, 1.0);
	
	gl_Position = position;
}

#type fragment
#version 410

uniform float u_NearPlane;
uniform float u_FarPlane;

void main()
{
	//gl_FragDepth = (1.0 / gl_FragCoord.z - 1.0 / u_NearPlane) / (1.0 / u_FarPlane - 1.0 / u_NearPlane);
}