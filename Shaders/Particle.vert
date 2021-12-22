#version 430 core
layout (location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 projection;

out vec4 colorPos;


void main()
{
	colorPos = model * vec4(aPos.xy, -3.0, 1.0);
	colorPos.z = -3.0;
	gl_Position = projection * colorPos;
}