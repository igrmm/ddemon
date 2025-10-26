#version 300 es
precision mediump float;

layout (location = 4) in vec2 is_p1;
layout (location = 5) in vec2 in_p0;
layout (location = 6) in vec2 in_p1;
layout (location = 7) in vec3 in_color;

out vec3 color;

void main()
{
    gl_Position.x = in_p0.x + in_p1.x * is_p1.x;
    gl_Position.y = in_p0.y + in_p1.y * is_p1.y;

    color = in_color;

    gl_Position.z = 0.0;
    gl_Position.w = 1.0;
}
