precision mediump float;

varying vec2 texture_coord_frag_0;
varying vec2 texture_coord_frag_1;
uniform sampler2D sampler_0;
uniform sampler2D sampler_1;
uniform float dx;
uniform float dy;
varying float weight_frag;

void main()
{
    vec2 flip_y = vec2(1.0, 1.0);
    float rate = 0.25;

    vec4 tex0 = texture2D(sampler_0, texture_coord_frag_0 * flip_y);
    vec4 tex1 = texture2D(sampler_0, (texture_coord_frag_0 + vec2(dx, 0)) * flip_y);
    vec4 tex2 = texture2D(sampler_0, (texture_coord_frag_0 + vec2(0, dy)) * flip_y);
    vec4 tex3 = texture2D(sampler_0, (texture_coord_frag_0 + vec2(dx, dy)) * flip_y);
    vec4 tex00 = texture2D(sampler_1, texture_coord_frag_1 * flip_y);
    vec4 tex11 = texture2D(sampler_1, (texture_coord_frag_1 + vec2(dx, 0)) * flip_y);
    vec4 tex22 = texture2D(sampler_1, (texture_coord_frag_1 + vec2(0, dy)) * flip_y);
    vec4 tex33 = texture2D(sampler_1, (texture_coord_frag_1 + vec2(dx, dy)) * flip_y);


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

    x = texture_coord_frag_1.x;
    y = texture_coord_frag_1.y;
    x0 = float(floor(x));
    y0 = float(floor(y));
    x1 = x0 + 1.0;
    y1 = y0 + 1.0;
    float weight_tex33 = (x1 - x) * (y1 - y);
    float weight_tex22 = (x - x0) * (y1 - y);
    float weight_tex11 = (x1 - x) * (y - y0);
    float weight_tex00 = 1.0 - weight_tex33 - weight_tex22 - weight_tex11;

    vec4 fusion0 = tex0 * weight_tex0 + tex1 * weight_tex1 + tex2 * weight_tex2 + tex3 * weight_tex3; 
    vec4 fusion1 = tex00 * weight_tex00 + tex11 * weight_tex11 + tex22 * weight_tex22 + tex33 * weight_tex33;
    
    vec4 tex = weight_frag * fusion0 + (1.0 - weight_frag) * fusion1;
    gl_FragColor = tex;
}