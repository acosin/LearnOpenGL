
#ifndef UTIL_H
#define UTIL_H

#ifndef debug_camera_renderer
#include <GLES2/gl2.h>

#else
#include <glad/glad.h>
#endif

#include <iostream>

#define TN_LOG(X) std::cout

namespace tn
{
namespace stitching
{

unsigned int CheckError(const char* file, int line);
unsigned int InnerCheckGLError(const char* file, int line);

#define CheckGLError(glFunc) \
    glFunc;\
    tn::stitching::InnerCheckGLError(__FILE__, __LINE__);
// #if defined ENABLE_GL_CHECK



} // namespace stitching {
} // namespace tn {

#endif // #ifndef UTIL_H
