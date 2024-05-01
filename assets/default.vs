#version 320 es
precision mediump float;
layout (location = 0) in vec4 vertex;
layout (location = 1) in vec4 offset;
out vec2 TexCoord;
void main()
{
    TexCoord = vertex.zw + offset.zw;
    gl_Position = vec4(vertex.x + offset.x, vertex.y + offset.y, 0, 1.0);
}
