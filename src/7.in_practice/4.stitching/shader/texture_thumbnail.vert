#version 310 es

layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoords;

out vec2 texture_coord_frag;

void main(){
    // gl_PointSize = 1.0;
    texture_coord_frag = aTexCoords;
    gl_Position = vec4(aPos, 0.0, 1.0); 
}
