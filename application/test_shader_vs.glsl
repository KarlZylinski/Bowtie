#version 410 core

in vec3 in_position;
out vec2 texcoord;

uniform mat4 model_view_projection_matrix;

void main()
{
    vec4 position4 = vec4(in_position, 1);
    texcoord = vec2(in_position.x, in_position.y);
    gl_Position = model_view_projection_matrix * position4;
}