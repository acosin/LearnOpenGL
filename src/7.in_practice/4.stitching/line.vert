#version 300 es
layout(location = 0) in vec2 aPos;    // 输入的顶点坐标
layout(location = 1) in vec3 aColor;  // 输入的颜色
out vec3 vColor;  // 传递到片段着色器的颜色
void main()
{
    gl_Position = vec4(aPos, 0.0, 1.0);
    vColor = aColor;
}