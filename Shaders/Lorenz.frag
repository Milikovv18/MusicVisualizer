#version 430 core
out vec4 fragColor;
in vec3 fragPos;

uniform bool colored;

void main()
{
	fragColor = colored ? vec4(fragPos, 1.0) : vec4(0.2);
}