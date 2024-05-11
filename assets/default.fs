#version 320 es
precision mediump float;

out vec4 frag_color;
in vec2 tex_coord;
uniform sampler2D atlas;

void main()
{
    frag_color = texture(atlas, tex_coord);
}
