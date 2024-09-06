#version 310 es

precision highp float;

layout (location = 0) in vec4 vertex;
out vec2 TexCoords;
uniform vec2 uImageBegin;
uniform vec2 uImageSize;

void main()
{
    vec2 point = vec2((uImageSize.x * vertex.x + uImageBegin.x),1.0 - (uImageSize.y * vertex.y + uImageBegin.y)) * 2.0 - 1.0;
    gl_Position = vec4(point,0.0,1.0);
    TexCoords = vertex.zw;
}  
