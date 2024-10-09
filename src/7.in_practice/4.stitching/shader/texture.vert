attribute vec2 aPosition;  						
attribute vec2 aTexture;
varying vec2 vTexture;
uniform vec2 uImageBegin;
uniform vec2 uImageSize;
void main(){
   vTexture = aTexture;
   vec2 point = vec2((uImageSize.x * aPosition.x + uImageBegin.x),1.0 - (uImageSize.y * aPosition.y + uImageBegin.y)) * 2.0 - 1.0;
   //gl_Position = vec4(point,0.0,1.0) * vec4(1.0, -1.0, 1.0, 1.0);
   gl_Position = vec4(point,0.0,1.0);
}
