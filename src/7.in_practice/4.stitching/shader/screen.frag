#version 310 es
precision mediump float;

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;

void main()
{
	vec2 tex_coord = TexCoords;
    vec3 col = texture(screenTexture, tex_coord).rgb;
    FragColor = vec4(col, 1.0); //in fact, output format is bgra in gl
} 