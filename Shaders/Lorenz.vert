#version 430 core
layout (location = 0) in vec3 aPos;

out vec3 fragPos;

uniform mat4 model;
uniform mat4 projection;


void main()
{
	gl_Position = projection * model * vec4(aPos, 1.0);
	fragPos = abs(vec3(model * vec4(aPos, 1.0)));
}