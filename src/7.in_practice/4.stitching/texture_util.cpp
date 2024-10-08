#include "texture_util.h"

#ifndef debug_camera_renderer
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include "tn_logger.h"
#else
#include <iostream>

#include <glad/glad.h>
#include "util.h"
#endif




#include "EGL/egl.h"
#include "IL/il.h"
#include "shader_util.h"
#include "config_manager.h"


namespace tn
{
namespace stitching
{


TextureUtil* TextureUtil::instance_ = nullptr;

TextureUtil::TextureUtil()
{
    ilInit();
    gl_enum_mapping_[ImageFormat::kRgb] = GL_RGB;
    gl_enum_mapping_[ImageFormat::kRgba] = GL_RGBA;
    gl_enum_mapping_[ImageFormat::kLuminance] = GL_LUMINANCE;
    gl_enum_mapping_[ImageFormat::kLuminanceAlpha] = GL_LUMINANCE_ALPHA;
    il_enum_mapping_[ImageFormat::kRgb] = IL_RGB;
    il_enum_mapping_[ImageFormat::kRgba] = IL_RGBA;
    gl_enum_mapping_[ImageFormat::kLuminance] = IL_LUMINANCE;
    gl_enum_mapping_[ImageFormat::kLuminanceAlpha] = IL_LUMINANCE_ALPHA;
    shader_program_ = 0;
    position_handle_ = 0;
    sample_handle_ = 0;
    texture_coord_handle_ = 0;
    image_size_handle_ = 0;
    image_begin_handle_ = 0;
    position_vbo_ = 0;
    index_vbo_ = 0;
}

TextureUtil::~TextureUtil()
{
    ilShutDown();
}

GLuint TextureUtil::GenerateTexture(GLenum target_type)
{
    GLuint texture = 0;
   CheckGLError(glGenTextures(1, &texture)); 
   CheckGLError(glBindTexture(target_type, texture));
   CheckGLError(glTexParameterf(target_type, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
   CheckGLError(glTexParameterf(target_type, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
   CheckGLError(glTexParameteri(target_type, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
   CheckGLError(glTexParameteri(target_type, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    //CheckGLError(glTexParameteri(target_type, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    //CheckGLError(glTexParameteri(target_type, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    return texture;
}

GLuint TextureUtil::LoadTexture(const std::string &texture_file, ImageFormat target_format, bool bilinear)
{
    GLuint texture_object = 0;
   CheckGLError(glGenTextures(1, &texture_object));
    bool bSuccess = UpdateTexture(texture_object, texture_file, target_format, bilinear);
    if(!bSuccess)
    {
        DeleteTexture(texture_object);
        texture_object = 0;
    }

    return texture_object;
}

GLuint TextureUtil::LoadTexture(int width, int height, unsigned char *buffer, ImageFormat source_format, ImageFormat target_format, bool bilinear)
{
    GLuint texture_object = 0;
   CheckGLError(glGenTextures(1, &texture_object));
    UpdateTexture(texture_object, width, height, buffer, source_format, target_format, bilinear);
    return texture_object;
}

void TextureUtil::DeleteTexture(GLuint& texture_id)
{
   CheckGLError(glDeleteTextures(1, &texture_id));
    texture_id = GL_NONE;
}

bool TextureUtil::UpdateTexture(GLuint texture_id, const std::string &texture_file, ImageFormat target_format, bool bilinear)
{
    ILuint image_id;
    ilGenImages(1, &image_id);
    ilBindImage(image_id); /* Binding of DevIL image name */
    ilEnable(IL_ORIGIN_SET);
    ilOriginFunc(IL_ORIGIN_LOWER_LEFT);
    ILboolean success = ilLoadImage((ILstring)(texture_file.c_str()));
    // auto test1 = ilDetermineType((ILstring)(texture_file.c_str()));
    // auto test2 = ilGetError();
    auto height = ilGetInteger(IL_IMAGE_HEIGHT);
    auto width = ilGetInteger(IL_IMAGE_WIDTH);
    if (success)
    {
        //std::cout << "TextureUtil::UpdateTexture texture_file " << texture_file << " height " << height << " width " << width << std::endl; // XW-11880 TEST
        TN_LOG(TN_LOG_ERROR) << "TextureUtil::UpdateTexture texture_file " << texture_file << " height " << height << " width " << width; // XW-11880 TEST
       CheckGLError(glActiveTexture(GL_TEXTURE0));
       CheckGLError(glBindTexture(GL_TEXTURE_2D, texture_id));
        ilConvertImage(il_enum_mapping_[target_format], IL_UNSIGNED_BYTE);
        // auto test3 = ilGetError();
        GLuint gl_format = gl_enum_mapping_[target_format];
        //CheckGLError(glTexImage2DMultisample(GL_TEXTURE_2D, 0, gl_format, width, height, 0, gl_format, GL_UNSIGNED_BYTE, ilGetData()));
       CheckGLError(glTexImage2D(GL_TEXTURE_2D, 0, gl_format, width, height, 0, gl_format, GL_UNSIGNED_BYTE, ilGetData()));
       CheckGLError(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
       if(bilinear)
       {
            CheckGLError(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR));
            CheckGLError(glGenerateMipmap(GL_TEXTURE_2D));
       }
       else
       {
            CheckGLError(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
       }
       CheckGLError(glBindTexture(GL_TEXTURE_2D, 0));
    }
    else
    {
        ILenum error = ilGetError();
        TN_LOG(TN_LOG_ERROR) << "error ilLoadImage: " << error << ", " << texture_file;
    }
    ilDeleteImages(1, &image_id);
    return success;

}

void TextureUtil::UpdateTexture(GLuint texture_id, int width, int height, unsigned char *buffer, ImageFormat source_format, ImageFormat target_format, bool bilinear)
{
    CheckGLError(glBindTexture(GL_TEXTURE_2D, texture_id));
    CheckGLError(glTexImage2D(GL_TEXTURE_2D, 0, gl_enum_mapping_[source_format], width, height, 0, gl_enum_mapping_[target_format], GL_UNSIGNED_BYTE, buffer));
    CheckGLError(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    if(bilinear)
    {
        CheckGLError(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR));
        CheckGLError(glGenerateMipmap(GL_TEXTURE_2D));
    }
    else
    {
        CheckGLError(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    }
}

void TextureUtil::ImageBlit(GLuint texture_id, float x, float y, float width, float height, bool left_top)
{
   CheckGLError(glUseProgram(shader_program_));
CheckGLError(glUniform2f(image_size_handle_, width, height));
CheckGLError(glUniform2f(image_begin_handle_, x, y));
CheckGLError(glActiveTexture (GL_TEXTURE0));
CheckGLError(glBindTexture(GL_TEXTURE_2D, texture_id));
CheckGLError(glUniform1i(sample_handle_, 0));
CheckGLError(glBindBuffer(GL_ARRAY_BUFFER, position_vbo_));
CheckGLError(glVertexAttribPointer(position_handle_, 2, GL_FLOAT, GL_FALSE, 0, 0));
CheckGLError(glEnableVertexAttribArray(position_handle_));
CheckGLError(glBindBuffer(GL_ARRAY_BUFFER, texture_vbo_[left_top]));
CheckGLError(glVertexAttribPointer(texture_coord_handle_, 2, GL_FLOAT, GL_FALSE, 0, 0));
CheckGLError(glEnableVertexAttribArray(texture_coord_handle_));
CheckGLError(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_vbo_));
CheckGLError(glEnable(GL_BLEND));
CheckGLError(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
CheckGLError(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, 0));
CheckGLError(glDisableVertexAttribArray(position_handle_));
CheckGLError(glDisableVertexAttribArray(texture_coord_handle_));
	//CheckGLError(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_NONE));
	//CheckGLError(glBindBuffer(GL_ARRAY_BUFFER, GL_NONE));
CheckGLError(glDisable(GL_BLEND));
    // GLuint dummy_vbo = GL_NONE;
	//CheckGLError(glGenBuffers(1, &dummy_vbo));
	//CheckGLError(glBindBuffer(GL_ARRAY_BUFFER, dummy_vbo));
}

bool TextureUtil::ImageBlitInitialized()
{
    return shader_program_ != 0;
}

void TextureUtil::InitImageBlit()
{
    if (init_done_)
        return;
    std::string shader_path = ConfigManager::Instance()->ShaderPath();
	if(tn::stitching::ShaderUtil::LoadShader(shader_path + "texture.vert", shader_path + "texture.frag", shader_program_) != 0)
		return;
	position_handle_ =CheckGLError(glGetAttribLocation(shader_program_, "aPosition"));
	sample_handle_ =CheckGLError(glGetUniformLocation(shader_program_,"sTexture"));
	texture_coord_handle_ =CheckGLError(glGetAttribLocation(shader_program_,"aTexture"));
	image_begin_handle_ =CheckGLError(glGetUniformLocation(shader_program_,"uImageBegin"));
	image_size_handle_ =CheckGLError(glGetUniformLocation(shader_program_,"uImageSize"));

    // std::cout << __func__ << "  ______  " << position_handle_ << " " << sample_handle_ << " " << texture_coord_handle_ << std::endl;

	float vertex_array[] = {0,0,0,1,1,0,1,1};   //vertex array,(0,0),(0,1),(1,0),(1,1) perspectively
	float left_bottom_texture_array[] = {0,1,0,0,1,1,1,0};   //texture array,flip the source image
	float left_top_texture_array[] = {0,0,0,1,1,0,1,1};   //texture array
	GLubyte index_array[] = {0, 1, 3,0, 3, 2};  //index array

CheckGLError(glGenBuffers(1, &position_vbo_));
CheckGLError(glBindBuffer(GL_ARRAY_BUFFER, position_vbo_));
CheckGLError(glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_array), vertex_array, GL_STATIC_DRAW));

CheckGLError(glGenBuffers(2, texture_vbo_));
CheckGLError(glBindBuffer(GL_ARRAY_BUFFER, texture_vbo_[0]));
CheckGLError(glBufferData(GL_ARRAY_BUFFER, sizeof(left_bottom_texture_array), left_bottom_texture_array, GL_STATIC_DRAW));

CheckGLError(glBindBuffer(GL_ARRAY_BUFFER, texture_vbo_[1]));
CheckGLError(glBufferData(GL_ARRAY_BUFFER, sizeof(left_top_texture_array), left_top_texture_array, GL_STATIC_DRAW));

CheckGLError(glGenBuffers(1, &index_vbo_));
CheckGLError(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_vbo_));
CheckGLError(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(index_array), index_array, GL_STATIC_DRAW));
    init_done_ = true;
	return;
}

void TextureUtil::DeinitImageBlit()
{
CheckGLError(glDeleteBuffers(1, &position_vbo_));
CheckGLError(glDeleteBuffers(2, texture_vbo_));
CheckGLError(glDeleteBuffers(1, &index_vbo_));
   CheckGLError(glDeleteProgram(shader_program_));
}

} // namespace stitching {
} // namespace tn {
