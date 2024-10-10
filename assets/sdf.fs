//https://libgdx.com/wiki/graphics/2d/fonts/distance-field-fonts
#version 320 es
precision mediump float;

out vec4 frag_color;
in vec2 tex_coord;
uniform sampler2D bmp;

const float smoothing = 3.0/16.0;

void main()
{
    float distance = texture(bmp, tex_coord).a;
    float alpha = smoothstep(0.5 - smoothing, 0.5 + smoothing, distance);
    frag_color = vec4(1.0, 1.0, 1.0, alpha);
}
