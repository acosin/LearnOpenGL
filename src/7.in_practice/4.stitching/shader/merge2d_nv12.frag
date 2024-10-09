
#extension GL_OES_EGL_image_external : require
precision highp float;

varying vec2 texture_coord_frag_0;
varying vec2 texture_coord_frag_1;

uniform sampler2D sampler_0;
uniform sampler2D sampler_00;
uniform sampler2D sampler_1;
uniform sampler2D sampler_11;

uniform float dx;
uniform float dy;
varying float weight_frag;

vec4 Nv12ToRgba2(sampler2D y_texture, sampler2D uv_texture, vec2 coord)
{
    vec3 yuv;
    yuv.x = texture2D(y_texture, coord).r;
    yuv.y = texture2D(uv_texture, coord).r - 0.5;
    yuv.z = texture2D(uv_texture, coord).a - 0.5;
    float r, g, b;

#define DIGITAL_YUV_601
#ifdef DIGITAL_YUV_601
    const float rangeY = 16.0 / 255.0;
    const float rangeU = 128.0 / 255.0;
    const float rangeV = 128.0 / 255.0;
    const vec3 yuv2r = vec3(1.164, 0.0, 1.596);
    const vec3 yuv2g = vec3(1.164, -0.392, -0.812);
    const vec3 yuv2b = vec3(1.164, 2.016, 0.0);
    const vec3 range = vec3(rangeY, rangeU, rangeV);
#else  // Analog
    const vec3 yuv2r = vec3(1.0, 0.0, 1.402);
    const vec3 yuv2g = vec3(1.0, -0.344, -0.792);
    const vec3 yuv2b = vec3(1.0, 1.772, 0.0);
    const vec3 range = vec3(0.0, 0.0, 0.0);
#endif

    yuv = yuv - range;
    r = dot(yuv, yuv2r);
    g = dot(yuv, yuv2g);
    b = dot(yuv, yuv2b);
    return vec4(r, g, b, 1.0);
}

vec4 Nv12ToRgba(sampler2D y_texture, sampler2D uv_texture, vec2 coord)
{
    vec3 yuv;
    yuv.x = texture2D(y_texture, coord).r;
    yuv.y = texture2D(uv_texture, coord).r - 0.5;
    yuv.z = texture2D(uv_texture, coord).a - 0.5;
    highp vec3 rgb = mat3(
        1.0, 1.0, 1.0,
        0.0, -0.344, 1.403,
        1.770, -0.714, 0.0) * yuv;
    return vec4(rgb, 1);    
}

void main()
{
    vec2 flip_y = vec2(1.0, 1.0);
    float rate = 0.25;

    vec4 tex0 = Nv12ToRgba(sampler_0, sampler_00, texture_coord_frag_0 * flip_y);
    vec4 tex1 = Nv12ToRgba(sampler_0, sampler_00, (texture_coord_frag_0 + vec2(dx, 0)) * flip_y);
    vec4 tex2 = Nv12ToRgba(sampler_0, sampler_00, (texture_coord_frag_0 + vec2(0, dy)) * flip_y);
    vec4 tex3 = Nv12ToRgba(sampler_0, sampler_00, (texture_coord_frag_0 + vec2(dx, dy)) * flip_y);
    vec4 tex00 = Nv12ToRgba(sampler_1, sampler_11, texture_coord_frag_1 * flip_y);
    vec4 tex11 = Nv12ToRgba(sampler_1, sampler_11, (texture_coord_frag_1 + vec2(dx, 0)) * flip_y);
    vec4 tex22 = Nv12ToRgba(sampler_1, sampler_11, (texture_coord_frag_1 + vec2(0, dy)) * flip_y);
    vec4 tex33 = Nv12ToRgba(sampler_1, sampler_11, (texture_coord_frag_1 + vec2(dx, dy)) * flip_y);

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