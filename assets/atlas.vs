#version 300 es
precision mediump float;

layout (location = 0) in vec2 is_width_or_height;
layout (location = 1) in vec4 instance_rect;
layout (location = 2) in vec4 texture_rect;

out vec2 tex_coord;

void main()
{
    //            |-position x-|   |----------------width---------------|
    tex_coord.x = texture_rect.x + texture_rect.z * is_width_or_height.x;
    //            |-position y-|   |----------------height--------------|
    tex_coord.y = texture_rect.y - texture_rect.w * is_width_or_height.y; //the minus "-" is because stbi loads image upside down

    //              |--position x-|   |----------------width----------------|
    gl_Position.x = instance_rect.x + instance_rect.z * is_width_or_height.x;
    //              |--position y-|   |----------------height---------------|
    gl_Position.y = instance_rect.y + instance_rect.w * is_width_or_height.y;

    gl_Position.z = 0.0;
    gl_Position.w = 1.0;
}
