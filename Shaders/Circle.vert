#version 430 core
layout (location = 0) in vec2 aPos;

out float scaling;

uniform float bassScale;
uniform mat4 model;
uniform mat4 projection;
uniform float scale[360];

// 180 vertices (180 is 80Hz)
void main()
{
	scaling = scale[gl_VertexID * 1 / 5] * 10.0f;
	gl_Position = projection * model * vec4(bassScale * (2.0f * scale[gl_VertexID * 1 / 5] + 1.0f) * aPos, -3.0, 1.0);
}