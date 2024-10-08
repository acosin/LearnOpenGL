/**
 * texture_util.h
 *
 * TextureUtil
 *
 * Created by jfzheng on 11/1/2020.
 * Copyright 2020 Telenav Inc. All rights reserved.
 *
 */
#ifndef TEXTURE_UTIL_H
#define TEXTURE_UTIL_H
#include <string>
#include <map>

#ifndef debug_camera_renderer
#include <GLES2/gl2.h>
#else
#include <glad/glad.h>
#endif

#include "IL/il.h"
#include "stitching_common.h"


namespace tn
{
namespace stitching
{

class TextureUtil
{
public:
    static TextureUtil* Instance() {
        if (nullptr == instance_)
            instance_ = new TextureUtil();
        return instance_;
    }
    GLuint GenerateTexture(GLenum target_type);
    GLuint LoadTexture(const std::string &texture_file, ImageFormat target_format, bool bilinear=true);
    GLuint LoadTexture(int width, int height, unsigned char *buffer, ImageFormat source_format, ImageFormat target_format, bool bilinear=true);
    void DeleteTexture(GLuint& texture_id);
    bool UpdateTexture(GLuint texture_id, const std::string &texture_file, ImageFormat target_format, bool bilinear=true);
    void UpdateTexture(GLuint texture_id, int width, int height, unsigned char *buffer, ImageFormat source_format, ImageFormat target_format, bool bilinear=true);
    void InitImageBlit();
    bool ImageBlitInitialized();
    void DeinitImageBlit();
    void ImageBlit(GLuint texture_id, float x, float y, float width, float height, bool left_top);
private:
    TextureUtil();
    ~TextureUtil();
    static TextureUtil* instance_;
    GLuint shader_program_;
    GLuint position_handle_;
    GLuint sample_handle_;
    GLuint texture_coord_handle_;
    GLuint image_size_handle_;
    GLuint image_begin_handle_;
    GLuint position_vbo_;
    GLuint texture_vbo_[2];
    GLuint index_vbo_;
    std::map<ImageFormat, GLuint> gl_enum_mapping_;
    std::map<ImageFormat, ILuint> il_enum_mapping_;
    bool init_done_ = false;
};

} // namespace stitching {
} // namespace tn {

#endif // #ifndef TEXTURE_UTIL_H