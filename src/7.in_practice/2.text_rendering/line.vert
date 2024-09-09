attribute vec2 aPos;   // 输入的顶点坐标
attribute vec3 aColor; // 输入的颜色
varying vec3 vColor;   // 传递到片段着色器的颜色
void main()
{
    gl_Position = vec4(aPos, 0.0, 1.0);
    vColor = aColor;
}