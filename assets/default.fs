#version 320 es
precision mediump float;

uniform sampler2D tex;

in vec2 tex_coord;
in vec3 tex_color;

out vec4 frag_color;

void main()
{
    frag_color = texture(tex, tex_coord) * vec4(tex_color, 1.0);
}
