/**
 * shader_util.h
 *
 * ShaderUtil
 *
 * Created by jfzheng on 11/1/2020.
 * Copyright 2020 Telenav Inc. All rights reserved.
 *
 */
#ifndef SHADER_UTIL_H
#define SHADER_UTIL_H

#ifndef debug_camera_renderer
#include <GLES2/gl2.h>
#else
#include <glad/glad.h>
#endif

#include <iostream>
#include <string>


namespace tn
{
namespace stitching
{

class ShaderUtil
{
public:
    static int LoadShader(const std::string &vertex_shader_file, const std::string &fragment_shader_file, GLuint &shader_program);
    static int CompileShader(const std::string &vertex_shader_src, const std::string &fragment_shader_src, GLuint &shader_program);
private:
    ShaderUtil() {}
    static int CompileShader(const std::string &shader_src, GLuint shader_id);
};





} // namespace stitching {
} // namespace tn {

#endif // #ifndef SHADER_UTIL_H