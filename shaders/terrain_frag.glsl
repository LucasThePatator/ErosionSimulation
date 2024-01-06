#version 330 core

in vec2 UV;
in vec3 position_Cam;
in vec3 normal_Cam;
in vec3 sun_Cam;

out vec3 color;

void main() 
{
	vec3 diffuseColor = vec3(231, 196, 150) / 255;
	vec3 specColor = vec3(231, 196, 231) / 255;

	float cos_i = clamp(dot(normal_Cam, sun_Cam), 0, 1);

	color = cos_i * diffuseColor;
	//color = sun_Cam;
}
