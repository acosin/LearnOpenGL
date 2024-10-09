#version 310 es
#extension GL_OES_EGL_image_external_essl3 : require
precision highp float;
out vec4 FragColor;

in vec2 texture_coord_frag;

uniform samplerExternalOES sTexture;

void main()
{
	vec2 flip_y = vec2(texture_coord_frag.x, 1.0 - texture_coord_frag.y);
    vec4 tex = texture(sTexture, flip_y);
    FragColor = tex;
}
