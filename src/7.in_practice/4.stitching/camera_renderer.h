/**
 * camera_renderer.h
 *
 * CameraRenderer
 *
 * Created by jfzheng on 11/1/2020.
 * Copyright 2020 Telenav Inc. All rights reserved.
 *
 */
#ifndef CAMERA_RENDERER_H
#define CAMERA_RENDERER_H
#include <vector>
#include <map>
#include <set>
#include <thread>
#include <atomic>
// #include "GLES3/gl3.h"

// #define TN_PLATFORM_LINUX_TDA4_AARCH64
#ifndef debug_camera_renderer
#include "GLES2/gl2.h"
#include "GLES2/gl2ext.h"
#else
#include <glad/glad.h>
#endif


#include "stitching_common.h"
#include "memory_pool.h"

#include "util.h"

namespace tn
{
namespace stitching
{

class CameraRenderer
{
public:
    struct ImageInput
    {
        ImageInput() {}
        ImageInput(uint8_t* d, int w, int h, ImageFormat f, Position a) : data(d), width(w), height(h), format(f), area(a) {}
        uint8_t* data = nullptr;;
        void* egl_img = nullptr;
        int width;
        int height;
        ImageFormat format;
        Position area;
    };
    CameraRenderer();
    ~CameraRenderer();
    void InitMassiveData();
    void InitGlRendering();
    void Init();
    void Render(RenderType type, GLenum texture_type, const std::map<Position, std::pair<GLuint, GLuint>>& textures);
    void Render(RenderType type, Position pos, GLenum texture_type, const std::map<Position, std::pair<GLuint, GLuint>>& textures);
    void RenderThumbnail(GLenum texture_type,
                         Position pos,
                         int org_width, int org_height,
                         int target_width, int target_height,
                         const std::map<Position, std::pair<GLuint, GLuint>>& textures);
    void SetInputImages(const std::vector<ImageInput>& input);
    void GetLeftTopInComposedCanvas(RenderType type, Position pos, int& left, int& top) const;
    
TN_PRIVATE
    enum class VboType{
        kVertex = 0,
        kTexture0,
        kTexture1,
        kIndex,
        kFusionWeight
    };
    enum class ShaderAttribute{
        kPosition = 0,
        kColor,
        kTextureCoord0,
        kTextureCoord1,
        kTextureSampler0,
        kTextureSampler00,
        kTextureSampler1,
        kTextureSampler11,
        kDx,
        kDy,
        kWeight,
        kTexuture1,
        kTransform
    };
    struct RenderParam
    {
        void CopyBoundry(const RenderParam& src) {
            opengl_render_x = src.opengl_render_x;
            opengl_render_y = src.opengl_render_y;
            opengl_render_width = src.opengl_render_width;
            opengl_render_height = src.opengl_render_height;
        }
        RenderType type = kInvalidType;
        Position position = kInvalidPosition;
        std::map<VboType, GLuint> vbo_table;
        std::vector<Position> lut_keys;
        // std::map<Position, GLuint> textures;
        GLuint shader_program = 0;
        int grid_per_row = 0;
        int grid_per_column = 0;
        int clipped_lut_width = 0;
        int clipped_lut_height = 0;
        float opengl_render_x = 0.0;
        float opengl_render_y = 0.0;
        float opengl_render_width = 0.0;
        float opengl_render_height = 0.0;
        int pixel_x_in_composed_canvas = 0;
        int pixel_y_in_composed_canvas = 0;
    };

    struct LutRenderData
    {
        LutRenderData() {}
        LutRenderData(const LutParam& p) : param(p) {}
        LutParam param;
        std::vector<unsigned char> coord_buff;
    };

    struct ThumbnailRenderParam
    {
        GLuint shader_program = 0;
        GLuint position_handle = 0;
        GLuint texture_coord_handle = 0;
        GLuint sample_handle = 0;
        GLuint vao_ = 0;

        bool IsValid() const
        {
            return ((shader_program != 0) &&
                    (vao_ != 0));
        }

        std::string ToString() const
        {
            std::string result;

            result += "\n\tshader_program - ";
            result += std::to_string(shader_program);

            result += "\n\tposition_handle - ";
            result += std::to_string(position_handle);

            result += "\n\ttexture_coord_handle - ";
            result += std::to_string(texture_coord_handle);

            result += "\n\tsample_handle - ";
            result += std::to_string(sample_handle);

            result += "\n\tvao_ - ";
            result += std::to_string(vao_);\

            return result;
        }
    };

    void InitLutParam();
    void InitTexture();
    void InitVbo();
    void InitParam();
    void InitGridParam();
    void InitRenderBoundries();
    void MapRenderBoundriesToComposedCanvas(std::map<RenderType, std::map<Position, RenderParam>>& data);
    void InitShaders();
    void InitRenderBuffers();
    void Uninit();
    void GenVbo();
    void GetLutOffset(RenderType type, Position render_pos, Position lut_pos, int& x_offset, int& y_offset);
    void RenderArea(const RenderParam& param, GLenum texture_type, const std::map<Position, std::pair<GLuint, GLuint>>& textures);
    void DrawTestGrids();
    bool IsOverlappedArea(Position area);
    void UpdateVbo(std::map<Position, RenderParam>& param_table);
    void Triangulate(GLfloat* addr, GLfloat x, GLfloat y, GLfloat x_jump, GLfloat y_jump, int dimension);
    void Tessellate(const RenderParam& param, std::vector<GLfloat, StitchingAllocator<GLfloat>>& vertex_buff);
    void LoadLut(const LutParam& param, std::vector<unsigned char>& coord);
    void LutToTextureBuff(const RenderParam& render_param, Position lut_key, std::vector<std::vector<GLfloat, StitchingAllocator<GLfloat>>>& dest_texture, std::vector<GLfloat, StitchingAllocator<GLfloat>>& dest_fusion);
    void RecollectMemory();
    void EnableGlAttribArray(GLuint vbo, GLuint index, GLuint size, GLenum data_type, GLuint stride);
    template<typename T> void SetGlBuffer(GLuint target, GLuint id, const std::vector<T, StitchingAllocator<T>>& data);
    std::vector<Position> position_list_;
    std::set<Position> overlapped_area_;
    std::map<Position, GLuint> textures_;
    std::map<RenderType, std::map<Position, RenderParam>> render_params_;
    std::map<GLuint, std::map<ShaderAttribute, GLuint>> shader_programs_;
    std::vector<Position> render_order_;
    std::map<RenderType, std::vector<LutRenderData>> lut_table_;
    std::map<RenderType, std::vector<std::vector<GLfloat, StitchingAllocator<GLfloat>>>> vertex_buff_;
    std::map<RenderType, std::vector<std::vector<std::vector<std::vector<GLfloat, StitchingAllocator<GLfloat>>>>>> texture_buff_;
    std::map<RenderType, std::vector<std::vector<std::vector<GLfloat, StitchingAllocator<GLfloat>>>>> fusion_weight_buff_;
    std::map<RenderType, std::vector<std::vector<GLuint, StitchingAllocator<GLuint>>>> index_buff_;
    const int vertex_dimension_ = 2;
    const int vertex_per_grid_ = 1; // render points
    float opengl_left_ = -1.0;
    float opengl_top_ = 1.0;
    const float opengl_max_width_ = 2.0;
    const float opengl_max_height_ = 2.0;
    ThumbnailRenderParam thumbnail_render_param_;
    // const int vertex_per_grid_ = 6; // render triangles
};


} // namespace stitching {
} // namespace tn {

#endif // #ifndef CAMERA_RENDERER_H