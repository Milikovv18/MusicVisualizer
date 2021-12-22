#version 430 core
in vec2 texCoords;
in float offsetX;

out vec4 fragColor;

uniform sampler2D arrowTex;
uniform vec3 color;
uniform float bassScale;

void main()
{
	fragColor = texture(arrowTex, texCoords) * (bassScale > 0.8 ? vec4(1.0) : vec4(color, 1.0));
    if(fragColor.a < 0.1f)
        discard;
}