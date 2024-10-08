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

#ifndef debug_font
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

unsigned int CheckError(const char* file, int line);
unsigned int InnerCheckGLError(const char* file, int line);

#define CheckGLError(glFunc) \
    glFunc;\
    tn::stitching::InnerCheckGLError(__FILE__, __LINE__);
// #if defined ENABLE_GL_CHECK


#ifdef debug_font
#define TN_LOG(X) std::cout
#endif

} // namespace stitching {
} // namespace tn {

#endif // #ifndef SHADER_UTIL_H