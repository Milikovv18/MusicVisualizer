#version 430 core
in float scaling;

out vec4 FragColor;


void main()
{
	FragColor = vec4(1.0, 1.0 - scaling, 1.0 - scaling, 0.0);
}