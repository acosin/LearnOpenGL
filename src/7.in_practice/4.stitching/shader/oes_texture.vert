attribute vec2 aPosition;
attribute vec2 aTexture;

varying vec2 texture_coord_frag;

void main(){
    gl_PointSize = 1.0;
    texture_coord_frag = aTexture;
    gl_Position = vec4(aPosition, 0.0, 1.0); 
}
