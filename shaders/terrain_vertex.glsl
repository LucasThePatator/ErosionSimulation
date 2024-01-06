#version 330 core

layout(location = 0) in vec3 vertexPosition_M;
layout(location = 1) in vec3 vertexNormal;
layout(location = 2) in vec2 vertexUV;

uniform mat4 V;
uniform mat4 MV;
uniform mat4 MVP;
uniform vec3 sun_position;

out vec2 UV;
out vec3 position_Cam;
out vec3 normal_Cam;
out vec3 sun_Cam;

void main()
{
	//TODO modify the height with a texture of the heightmap instead

	position_Cam = (MV * vec4(vertexPosition_M, 1)).xyz;
	normal_Cam = (MV * vec4(vertexNormal, 0)).xyz;
	sun_Cam = (V * vec4(sun_position, 0)).xyz;

	gl_Position = MVP * vec4(vertexPosition_M, 1);
	UV = vertexUV;
}