#version 410 core

in vec2 texcoord;

uniform sampler2D texture_sampler;

out vec4 color;

void main()
{
	color = texture(texture_sampler, texcoord);
}