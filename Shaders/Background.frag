#version 430 core
in vec2 texCoords;

out vec4 fragColor;

uniform sampler2D picture;

void main()
{
	fragColor = texture(picture, texCoords);
	if (fragColor.a < 0.1f)
		discard;
}