#version 410 core

in vec3 in_position;
out vec2 texcoord;

void main()
{
    texcoord = (in_position.xy + vec2(1,1)) * 0.5;
    gl_Position = vec4(in_position, 1);
}

#fragment
#version 410 core

in vec2 texcoord;

uniform sampler2D texture_samplers[16];
uniform int num_samplers;

out vec4 color;

void main()
{
	color = vec4(0, 0, 0, 0);

	for (int i = 0; i < num_samplers; ++i) {
		color += color.a * color + (1 - color.a) * texture(texture_samplers[i], texcoord);
	}
}
