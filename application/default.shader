#version 410 core

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec2 in_texcoord;
out vec2 texcoord;

uniform mat4 model_view_projection_matrix;

void main()
{
    vec4 position4 = vec4(in_position, 1);
    texcoord = in_texcoord;
    gl_Position = model_view_projection_matrix * position4;
}

#fragment
#version 410 core

in vec2 texcoord;

uniform sampler2D texture_sampler;

out vec4 color;

void main()
{
	color = texture(texture_sampler, texcoord);
}