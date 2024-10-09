#extension GL_OES_EGL_image_external : require
precision highp float;


uniform samplerExternalOES sampler_0;
uniform samplerExternalOES sampler_00;

varying vec3 color_frag;
varying vec2 vTexCoord;
varying vec2 texture_coord_frag_0;
uniform float dx;
uniform float dy;

varying vec4 multi_sampler_weight_0;

vec4 Nv12ToRgba2(samplerExternalOES y_texture, samplerExternalOES uv_texture, vec2 coord)
{
    vec3 yuv;
    yuv.x = texture2D(y_texture, coord).r;
    yuv.y = texture2D(uv_texture, coord).r;
    yuv.z = texture2D(uv_texture, coord).a;
    float r, g, b;

// #define DIGITAL_YUV_601
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

vec4 Nv12ToRgba(samplerExternalOES y_texture, samplerExternalOES uv_texture, vec2 coord)
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