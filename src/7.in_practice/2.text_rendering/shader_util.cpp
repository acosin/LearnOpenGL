#include "shader_util.h"

#include <vector>
#include <fstream>
#include <sstream>
#ifndef debug_font
#include "tn_logger.h"

#include "stitching_common.h"
#endif

namespace tn
{
namespace stitching
{


int ShaderUtil::LoadShader(const std::string &vertex_shader_file, const std::string &fragment_shader_file, GLuint &shader_program)
{
    // TN_LOG(TN_LOG_INFO) << "Compiling shader : " << vertex_shader_file << ", " << fragment_shader_file;
    // std::cout  << "Compiling shader : " << vertex_shader_file << ", " << fragment_shader_file << std::endl;
    std::string v_src;
    std::string f_src;
    std::ifstream v_stream(vertex_shader_file, std::ios::in);
    std::ifstream f_stream(fragment_shader_file, std::ios::in);
    if(v_stream.is_open() && f_stream.is_open())
    {
        std::stringstream sstr;
        sstr << v_stream.rdbuf();
        v_src = sstr.str();
        v_stream.close();
        sstr.str("");
        sstr << f_stream.rdbuf();
        f_src = sstr.str();
        f_stream.close();
    }
    else
        return -1;
    return CompileShader(v_src, f_src, shader_program);
}

int ShaderUtil::CompileShader(const std::string &vertex_shader_src, const std::string &fragment_shader_src, GLuint &shader_program)
{
    GLuint vertex_shader_id =CheckGLError(glCreateShader(GL_VERTEX_SHADER));
    GLuint fragment_shader_id =CheckGLError(glCreateShader(GL_FRAGMENT_SHADER));
    if (CompileShader(vertex_shader_src, vertex_shader_id) != 0
    || CompileShader(fragment_shader_src, fragment_shader_id) != 0)
        return -1;

    shader_program =CheckGLError(glCreateProgram());

   CheckGLError(glAttachShader(shader_program , vertex_shader_id));
   CheckGLError(glAttachShader(shader_program , fragment_shader_id));

   CheckGLError(glLinkProgram(shader_program));
    GLint success = false;
   CheckGLError(glGetProgramiv(shader_program , GL_LINK_STATUS, &success));
    if (!success)
    {
        GLint info_log_length, errorLength;
       CheckGLError(glGetProgramiv(shader_program , GL_INFO_LOG_LENGTH, &info_log_length));

        if (info_log_length > 0)
        {
            std::vector<char> error_message(info_log_length + 1);
           CheckGLError(glGetProgramInfoLog(shader_program, info_log_length, NULL, &error_message[0]));
            TN_LOG(TN_LOG_ERROR) << &error_message[0];
        }
        TN_LOG(TN_LOG_ERROR) << "shader program link failed";
        return -2;
    }

   CheckGLError(glDetachShader(shader_program, vertex_shader_id));
   CheckGLError(glDetachShader(shader_program, fragment_shader_id));

   CheckGLError(glDeleteShader(vertex_shader_id));
   CheckGLError(glDeleteShader(fragment_shader_id));
    return 0;
}

int ShaderUtil::CompileShader(const std::string &shader_src, GLuint shader_id)
{
    // TN_LOG(TN_LOG_INFO) << "compile shader:" << shader_src << std::endl;
    char const * shader_ptr = shader_src.c_str();
    GLint length = shader_src.size();
   CheckGLError(glShaderSource(shader_id, 1, &shader_ptr , &length));
   CheckGLError(glCompileShader(shader_id));

    GLint Result = GL_FALSE;
    int info_log_length;
   CheckGLError(glGetShaderiv(shader_id, GL_COMPILE_STATUS, &Result));
   CheckGLError(glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &info_log_length));
    if (Result != GL_TRUE)
    {
        std::vector<char> error_message(info_log_length + 1);
       CheckGLError(glGetShaderInfoLog(shader_id, info_log_length, NULL, &error_message[0]));
        TN_LOG(TN_LOG_ERROR) << &error_message[0];
        return -2;
    }

    return 0;
}

} // namespace stitching {
} // namespace tn {