#version 320 es
precision mediump float;
out vec4 FragColor;
in vec2 TexCoord;
uniform sampler2D atlas;
void main()
{
    FragColor = texture(atlas, TexCoord);
}
