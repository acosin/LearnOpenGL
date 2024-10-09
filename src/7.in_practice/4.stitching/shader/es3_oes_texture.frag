#version 310 es
#extension GL_OES_EGL_image_external_essl3 : require
precision highp float;
out vec4 FragColor;

in vec2 TexCoord;

// texture sampler
uniform samplerExternalOES texture1;
uniform float middle_x;
uniform float middle_y;

void main()
{
	vec3 col = texture(texture1, TexCoord).rgb;
	float width = 0.001;
	if(TexCoord.x > middle_x-width && TexCoord.x < middle_x+width)
	{
		col = vec3(1.0, 0.0, 0.0);
	}
	if(TexCoord.y > middle_y-width && TexCoord.y < middle_y+width)
	{
		col = vec3(1.0, 0.0, 0.0);
	}
    FragColor = vec4(col.r, col.g, col.b, 1.0);   // output format is rgba in gl
}
