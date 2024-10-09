precision highp float;
uniform sampler2D sampler;
varying vec2 texture_coord_frag;

void main()                         
{
    vec2 flip_y = vec2(1.0, -1.0);

    // vec2 blur_fact = vec2(0.00142, 0.00125);
    vec2 blur_fact = vec2(0.00284, 0.00250);
    // vec2 blur_fact = vec2(0.00182, 0.00185);
    blur_fact = blur_fact * 0.8;
    vec4 tex = texture2D(sampler, texture_coord_frag * flip_y);
    vec4 tex0 = texture2D(sampler, (texture_coord_frag + blur_fact) * flip_y);
    vec4 tex1 = texture2D(sampler, (texture_coord_frag + blur_fact * vec2(-1.0, 1.0)) * flip_y);
    vec4 tex2 = texture2D(sampler, (texture_coord_frag + blur_fact * vec2(1.0, -1.0)) * flip_y);
    vec4 tex3 = texture2D(sampler, (texture_coord_frag + blur_fact * vec2(-1.0, -1.0)) * flip_y);


    gl_FragColor = tex * 0.5 + 0.5 * (tex0 + tex1 + tex2 + tex3) / 4.0;
    // gl_FragColor = tex;
    //gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
}  
