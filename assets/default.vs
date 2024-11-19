#version 300 es
precision mediump float;

layout (location = 0) in vec2 is_width_or_height;
layout (location = 1) in vec4 in_dst_rect;
layout (location = 2) in vec4 in_src_rect;
layout (location = 3) in vec3 in_color;

out vec2 tex_coord;
out vec3 tex_color;

void main()
{
    //            |-positionx-|   |---------------width---------------|
    tex_coord.x = in_src_rect.x + in_src_rect.z * is_width_or_height.x;
    //            |-positiony-|   |---------------height--------------|
    tex_coord.y = in_src_rect.y + in_src_rect.w * is_width_or_height.y;

    //              |-positionx-|   |---------------width---------------|
    gl_Position.x = in_dst_rect.x + in_dst_rect.z * is_width_or_height.x;
    //              |-positiony-|   |---------------height--------------|
    gl_Position.y = in_dst_rect.y + in_dst_rect.w * is_width_or_height.y;

    tex_color = in_color;

    gl_Position.z = 0.0;
    gl_Position.w = 1.0;
}
