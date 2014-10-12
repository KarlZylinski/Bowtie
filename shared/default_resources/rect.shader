#version 410 core

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec2 in_texcoord;
layout(location = 2) in vec4 in_color;
out vec2 texcoord;
out vec4 vertex_color;

uniform mat4 model_view_projection_matrix;

void main()
{
    vec4 position4 = vec4(in_position, 1);
    texcoord = in_texcoord;
    vertex_color = in_color;
    gl_Position = model_view_projection_matrix * position4;
}

#fragment
#version 410 core

in vec2 texcoord;
in vec4 vertex_color;

layout(location = 0) out vec4 color;

void main()
{
    color = vertex_color;
}
