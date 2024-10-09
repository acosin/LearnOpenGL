uniform mat4  uProjMatrix;

attribute vec3 position;
attribute vec2 texture_coord_vert_0;
attribute vec2 texture_coord_vert_1;
varying vec2 texture_coord_frag_0;
varying vec2 texture_coord_frag_1;
attribute float weight_vert;
varying float weight_frag;

void main(){
    gl_PointSize = 1.0;
    texture_coord_frag_0 = texture_coord_vert_0;
    texture_coord_frag_1 = texture_coord_vert_1;
    weight_frag = weight_vert;
    gl_Position = vec4(position,1.0) * vec4(1.0, -1.0, 1.0, 1.0); 
}
