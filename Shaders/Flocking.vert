#version 430 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTex;

out vec2 texCoords;
out float offsetX;

uniform mat4 model;
uniform mat4 projection;

void main()
{
	gl_Position = projection * model * vec4(aPos, 1.0);
	texCoords = aTex;
	offsetX = gl_Position.x;
}