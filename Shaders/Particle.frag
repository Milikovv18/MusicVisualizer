#version 430 core
out vec4 fragColor;

in vec4 colorPos;

void main()
{
	fragColor = abs(colorPos);
}