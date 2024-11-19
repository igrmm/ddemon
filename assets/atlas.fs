#version 300 es
precision mediump float;

out vec4 frag_color;
in vec2 tex_coord;
uniform sampler2D atlas;

void main()
{
    frag_color = texture(atlas, tex_coord);

    // this step is necessary to preserve alpha channel because we create atlas
    // texture with offscreen rendering, see
    // https://stackoverflow.com/questions/24346585/opengl-render-to-texture-with-partial-transparancy-translucency-and-then-rende
    frag_color.rgb *= frag_color.a;
}
