


#include "util.h"

#include <cstdio>

namespace tn
{
namespace stitching
{

const char *opengl_error_string(GLenum flag)
{
    static char str[256] = {'\0'};

    switch (flag)
    {
    case GL_NO_ERROR:
        snprintf(str, sizeof(str), "0x%04x:%s", flag, "GL_NO_ERROR");
        break;
    case GL_INVALID_ENUM:
        snprintf(str, sizeof(str), "0x%04x:%s", flag, "GL_INVALID_ENUM");
        break;
    case GL_INVALID_VALUE:
        snprintf(str, sizeof(str), "0x%04x:%s", flag, "GL_INVALID_VALUE");
        break;
    case GL_INVALID_OPERATION:
        snprintf(str, sizeof(str), "0x%04x:%s", flag, "GL_INVALID_OPERATION");
        break;
    case GL_INVALID_FRAMEBUFFER_OPERATION:
        snprintf(str, sizeof(str), "0x%04x:%s", flag, "GL_INVALID_FRAMEBUFFER_OPERATION");
        break;
    case GL_OUT_OF_MEMORY:
        snprintf(str, sizeof(str), "0x%04x:%s", flag, "GL_OUT_OF_MEMORY");
        break;
    default:
        snprintf(str, sizeof(str), "unknown flag:0x%04x", flag);
    }

    return str;
}


unsigned int CheckError(const char* file, int line)
{
    GLenum error_code =glGetError();
    if (error_code != GL_NO_ERROR)
    {
        TN_LOG(TN_LOG_ERROR) << "ERROR:" << file << ":" << line << " ==>" << opengl_error_string(error_code) <<  std::endl;
    }
    // else
    //    TN_LOG(TN_LOG_ERROR) << "ERROR:" << file << ":" << line << " ==>" << ErrorDescription(error_code) <<  std::endl;
    return error_code;
}


unsigned int InnerCheckGLError(const char* file, int line)
{
    return CheckError(file, line);
}


} // namespace stitching {
} // namespace tn {
