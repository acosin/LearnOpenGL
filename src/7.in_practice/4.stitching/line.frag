#version 300 es
precision mediump float;
in vec3 vColor;    // 从顶点着色器传入的颜色
out vec4 FragColor;
void main()
{
   FragColor = vec4(vColor, 1.0);  // 设置输出颜色
}