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

uniform sampler2D texture_sampler1;
uniform sampler2D texture_sampler2;

out vec4 color;

void main()
{
	color = texture(texture_sampler1, texcoord) + texture(texture_sampler2, texcoord);
}
