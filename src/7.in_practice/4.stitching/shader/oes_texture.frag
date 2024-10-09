#extension GL_OES_EGL_image_external : require
precision highp float;

varying vec2 texture_coord_frag;

uniform samplerExternalOES sTexture;

void main()
{
    vec4 tex = texture2D(sTexture, texture_coord_frag);
    gl_FragColor = tex;
}