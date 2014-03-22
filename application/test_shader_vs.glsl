#version 410 core

in vec3 in_position;

uniform mat4 model_view_projection_matrix;

void main()
{
    vec4 position4 = vec4(in_position, 1);
    gl_Position = model_view_projection_matrix * position4;
}