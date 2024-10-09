attribute vec3 position;
attribute vec2 texture_coord_vert_0;
varying vec2 texture_coord_frag_0;

void main(){
    gl_PointSize = 1.0;
    texture_coord_frag_0 = texture_coord_vert_0;
    gl_Position = vec4(position, 1.0)  * vec4(1.0, -1.0, 1.0, 1.0); 	
}
