#version 410 core

in vec2 texcoord;

uniform sampler2D texture_sampler;

out vec4 color;

void main()
{
	vec4 tex = texture(texture_sampler, texcoord);

	if (tex.a == 0)
		discard;

	color = tex;
}