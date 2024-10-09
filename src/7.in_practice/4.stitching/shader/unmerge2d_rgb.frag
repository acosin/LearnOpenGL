precision mediump float;
uniform sampler2D sampler_0;
varying vec3 color_frag;
varying vec2 vTexCoord;
varying vec2 texture_coord_frag_0;
uniform float dx;
uniform float dy;

varying vec4 multi_sampler_weight_0;
void main()                         
{
    vec2 flip_y = vec2(1.0, 1.0);

    vec4 tex0 = texture2D(sampler_0, texture_coord_frag_0 * flip_y);
    vec4 tex1 = texture2D(sampler_0, (texture_coord_frag_0 + vec2(dx, 0)) * flip_y);
    vec4 tex2 = texture2D(sampler_0, (texture_coord_frag_0 + vec2(0, dy)) * flip_y);
    vec4 tex3 = texture2D(sampler_0, (texture_coord_frag_0 + vec2(dx, dy)) * flip_y);

    float x = texture_coord_frag_0.x;
    float y = texture_coord_frag_0.y;
    float x0 = float(floor(x));
    float y0 = float(floor(y));
    float x1 = x0 + 1.0;
    float y1 = y0 + 1.0;
    float weight_tex3 = (x1 - x) * (y1 - y);
    float weight_tex2 = (x - x0) * (y1 - y);
    float weight_tex1 = (x1 - x) * (y - y0);
    float weight_tex0 = 1.0 - weight_tex3 - weight_tex2 - weight_tex1;

    gl_FragColor = tex0 * weight_tex0 + tex1 * weight_tex1 + tex2 * weight_tex2 + tex3 * weight_tex3; 
}