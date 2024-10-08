#include "camera_renderer.h"

#ifdef TN_PLATFORM_LINUX_TDA4_AARCH64
    #define GL_GLEXT_PROTOTYPES
    #include "GLES3/gl31.h"
#endif  // TN_PLATFORM_LINUX_TDA4_AARCH64

#if debug_camera_renderer
#include "number_packer.h"
#else
#include "tn/stitching_common/number_packer.h"
#include "tn_logger.h"
#endif

#include <vector>
#include <map>
#include <fstream>
#include <iostream>
#include <iterator>
#include <iomanip>

#include "texture_util.h"
#include "shader_util.h"
#include "config_manager.h"

#include "util.h"



namespace tn
{
namespace stitching
{

// 3 floats: x,y,coeff
struct BevNumPackIndex {
static constexpr int X = 0;
static constexpr int Y = 1;
static constexpr int Coeff = 2;
static constexpr int Count = 3;
};

struct PerspectNumPackIndex {
static constexpr int X = 0;
static constexpr int Y = 1;
static constexpr int Count = 2;
};

constexpr int kInterpWeightCount = 4; 

static const tn::stitching::NumberPacker<float> g_float32_packer;

static inline float ReadFloat(const uint32_t buf)
{
    return g_float32_packer.FromBuffer((uint8_t*)&buf);
}

void BilinearInterpolation(float pixel[2], float weight[kInterpWeightCount])
{

    float y = pixel[0];
    float x = pixel[1];

    int x0 = (int)x;
    int x1 = (int)x + 1;
    int y0 = (int)y;
    int y1 = (int)y + 1;

    x0 = x0 < 0 ? 0 : x0;
    x1 = x1 < 0 ? 0 : x1;
    y0 = y0 < 0 ? 0 : y0;
    y1 = y1 < 0 ? 0 : y1;

    float weight_up_left = (x1 - x) * (y1 - y);
    float weight_up_right = (x - x0) * (y1 - y);
    float weight_down_left = (x1 - x) * (y - y0);
    float weight_down_right = 1 - weight_up_left - weight_up_right - weight_down_left;

    weight[0] = weight_up_left;
    weight[1] = weight_up_right;
    weight[2] = weight_down_left;
    weight[3] = weight_down_right;
}

static inline void FillSampleWeight(
    std::vector<GLfloat, StitchingAllocator<GLfloat>>& dst_multisample, int samp_idx, uint32_t* raw_coord, int grid_index, bool is_bev)
{
    float weight[kInterpWeightCount];

    const int kPackCount = is_bev ? BevNumPackIndex::Count : PerspectNumPackIndex::Count;
    const int kPackX = is_bev ? BevNumPackIndex::X : PerspectNumPackIndex::X;
    const int kPackY = is_bev ? BevNumPackIndex::Y : PerspectNumPackIndex::Y;

    float pixel_coordinate[2] = {
        ReadFloat(raw_coord[grid_index * kPackCount + kPackX]), 
        ReadFloat(raw_coord[grid_index * kPackCount + kPackY])
    };
    BilinearInterpolation(pixel_coordinate, weight);
    for (int k = 0; k < kInterpWeightCount; ++k)
    {
        dst_multisample[samp_idx + k] = (float)weight[k];
    }
}

static inline void FillTextureCoord(
    std::vector<std::vector<GLfloat, StitchingAllocator<GLfloat>>>& dst_texture, int txt_idx, uint32_t* raw_coord, int grid_index,
    const LutParam& param, bool is_bev)
{
    const int kPackCount = is_bev ? BevNumPackIndex::Count : PerspectNumPackIndex::Count;
    const int kPackX = is_bev ? BevNumPackIndex::X : PerspectNumPackIndex::X;
    const int kPackY = is_bev ? BevNumPackIndex::Y : PerspectNumPackIndex::Y;

    dst_texture[0][txt_idx] = 
        (GLfloat)ReadFloat(raw_coord[grid_index * kPackCount + kPackX]) / param.src_image_width;
    dst_texture[0][txt_idx + 1] = 
        (GLfloat)ReadFloat(raw_coord[grid_index * kPackCount + kPackY]) / param.src_image_height;
}

CameraRenderer::CameraRenderer()
{
}

CameraRenderer::~CameraRenderer()
{
    Uninit();
}

void CameraRenderer::EnableGlAttribArray(GLuint vbo, GLuint index, GLuint size, GLenum data_type, GLuint stride)
{
   CheckGLError(glBindBuffer(GL_ARRAY_BUFFER, vbo));
   CheckGLError(glVertexAttribPointer(index, size, data_type, GL_FALSE, stride, 0));
   CheckGLError(glEnableVertexAttribArray(index));
}

void CameraRenderer::DrawTestGrids()
{
    std::string shader_path = ConfigManager::Instance()->ShaderPath();
    GLuint debugging_shader_program;
    if (0 != ShaderUtil::LoadShader(shader_path + "/debugging.vert", shader_path + "/debugging.frag", debugging_shader_program))
        return;
    GLuint attr =CheckGLError(glGetAttribLocation(debugging_shader_program, "position"));
   CheckGLError(glUseProgram(debugging_shader_program));
   CheckGLError(glLineWidth(2));
    for (auto& param : render_params_[RenderType::kFarAvm])
    {
        if (!IsOverlappedArea(param.first))
            continue;
        if (!ConfigManager::Instance()->RenderEnabled(param.first))
            continue;
        EnableGlAttribArray(param.second.vbo_table.at(VboType::kVertex), attr, vertex_dimension_, GL_FLOAT, 0);
        //CheckGLError(glDrawArrays(GL_POINTS, 0, param.second.grid_per_row * param.second.grid_per_column));
       CheckGLError(glDisableVertexAttribArray(attr));
    }
    static GLuint vbo = -1;
    if (-1 == vbo)
    {
        GLfloat vtx[] = {-1.0, 1.0, 0.0, 1.0, -1.0, 0.0, -0.5, 0.0, 0.0};
       CheckGLError(glGenBuffers(1, &vbo));
       CheckGLError(glBindBuffer(GL_ARRAY_BUFFER, vbo));
       CheckGLError(glBufferData(GL_ARRAY_BUFFER, 9 * sizeof(GLfloat), vtx, GL_STATIC_DRAW));
    }
    EnableGlAttribArray(vbo, attr, 3, GL_FLOAT, 0);
   CheckGLError(glDrawArrays(GL_LINE_STRIP, 0, 3));
   CheckGLError(glDisableVertexAttribArray(attr));
}

void CameraRenderer::RenderArea(const RenderParam& param, GLenum texture_type, const std::map<Position, std::pair<GLuint, GLuint>>& textures)
{
    auto program = shader_programs_[param.shader_program];
    float dx = 1.0 / ConfigManager::Instance()->InputImageWidth();
    float dy = 1.0 / ConfigManager::Instance()->InputImageHeight();
    // float dx = 0.5;
    // float dy = 0.5;
    bool nv12_texture = ConfigManager::Instance()->InputImageFormat() == ImageFormat::kNv12;
    // std::cout << "cr nv: " << nv12_texture << std::endl;

   CheckGLError(glUseProgram(param.shader_program));
   CheckGLError(glActiveTexture(GL_TEXTURE0));
   CheckGLError(glBindTexture(texture_type, textures.at(param.lut_keys[0]).first));
    // TODO: change to glProgramUniform, and reduce shader variable
   CheckGLError(glUniform1i(shader_programs_[param.shader_program][ShaderAttribute::kTextureSampler0], 0));
    if (nv12_texture && textures.at(param.lut_keys[0]).second != 0)
    {
       CheckGLError(glActiveTexture(GL_TEXTURE1));
       CheckGLError(glBindTexture(texture_type, textures.at(param.lut_keys[0]).second));
       CheckGLError(glUniform1i(shader_programs_[param.shader_program][ShaderAttribute::kTextureSampler00], 1));
    }
   CheckGLError(glUniform1f(shader_programs_[param.shader_program][ShaderAttribute::kDx], dx));
   CheckGLError(glUniform1f(shader_programs_[param.shader_program][ShaderAttribute::kDy], dy));

    EnableGlAttribArray(param.vbo_table.at(VboType::kTexture0), program[ShaderAttribute::kTextureCoord0], 2, GL_FLOAT, 0);

    bool two_textures = param.lut_keys.size() == 2;
    if (two_textures)
    {
        if (nv12_texture && textures.at(param.lut_keys[1]).second != 0)
        {
           CheckGLError(glActiveTexture(GL_TEXTURE2));
           CheckGLError(glBindTexture(texture_type, textures.at(param.lut_keys[1]).first));
           CheckGLError(glUniform1i(shader_programs_[param.shader_program][ShaderAttribute::kTextureSampler1], 2));
           CheckGLError(glActiveTexture(GL_TEXTURE3));
           CheckGLError(glBindTexture(texture_type, textures.at(param.lut_keys[1]).second));
           CheckGLError(glUniform1i(shader_programs_[param.shader_program][ShaderAttribute::kTextureSampler11], 3));
        }
        else
        {
           CheckGLError(glActiveTexture(GL_TEXTURE1));
           CheckGLError(glBindTexture(texture_type, textures.at(param.lut_keys[1]).first));
           CheckGLError(glUniform1i(shader_programs_[param.shader_program][ShaderAttribute::kTextureSampler1], 1));
        }

        EnableGlAttribArray(param.vbo_table.at(VboType::kFusionWeight), program[ShaderAttribute::kWeight], 1, GL_FLOAT, 0);

        EnableGlAttribArray(param.vbo_table.at(VboType::kTexture1), program[ShaderAttribute::kTextureCoord1], 2, GL_FLOAT, 0);
    }
    EnableGlAttribArray(param.vbo_table.at(VboType::kVertex), program[ShaderAttribute::kPosition], vertex_dimension_, GL_FLOAT, 0); //sizeof ( GLfloat )*3);
    //CheckGLError(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, param.vbo_table.at(VboType::kIndex)));
    //CheckGLError(glDrawElements(GL_POINTS, param.grid_per_row * param.grid_per_column, GL_UNSIGNED_INT, 0));
    //CheckGLError(glDrawElements(GL_POINTS, param.grid_per_row * param.grid_per_column, GL_UNSIGNED_SHORT, 0));
   CheckGLError(glDrawArrays(GL_POINTS, 0,  param.grid_per_row * param.grid_per_column));

   CheckGLError(glDisableVertexAttribArray(program[ShaderAttribute::kPosition]));
   CheckGLError(glDisableVertexAttribArray(program[ShaderAttribute::kTextureCoord0]));
    if (two_textures)
    {
       CheckGLError(glDisableVertexAttribArray(program[ShaderAttribute::kTextureCoord1]));
       CheckGLError(glDisableVertexAttribArray(program[ShaderAttribute::kWeight]));
    }
}

bool CameraRenderer::IsOverlappedArea(Position area)
{
    return overlapped_area_.end() != overlapped_area_.find(area);
}

void CameraRenderer::Render(RenderType type, GLenum texture_type, const std::map<Position, std::pair<GLuint, GLuint>>& textures)
{
    auto start = std::chrono::steady_clock::now();
    for (auto& p : render_params_[type])
    {
        if (ConfigManager::Instance()->RenderEnabled(p.first))
            RenderArea(p.second, texture_type, textures);
    }
    // printf("___________ CameraRenderer::Render cost: %dms\n", std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count());
    // DrawTestGrids();
}

void CameraRenderer::Render(RenderType type, Position pos, GLenum texture_type, const std::map<Position, std::pair<GLuint, GLuint>>& textures)
{
    RenderArea(render_params_[type][pos], texture_type, textures);
}

void CameraRenderer::RenderThumbnail(GLenum texture_type,
                                     Position pos,
                                     int org_width, int org_height,
                                     int target_width, int target_height,
                                     const std::map<Position, std::pair<GLuint, GLuint>>& textures)
{
    if (!ConfigManager::Instance()->GenerateSurroundThumnails())
    {
        return ;
    }

    if (texture_type != GL_TEXTURE_EXTERNAL_OES)
    {
        TN_LOG(TN_LOG_VERBOSE) << "Now only support oes texture";
        return ;
    }
#if defined(TN_PLATFORM_LINUX_TDA4_AARCH64)
    if (org_width == 0 ||
        org_height == 0 ||
        target_width == 0 ||
        target_height == 0)
    {
        TN_LOG(TN_LOG_INFO) << "Invalid dims";
        return ;
    }

    const auto& input_textures = textures.at(pos);

    if (input_textures.first == 0)
    {
        TN_LOG(TN_LOG_INFO) << "No valid input textures";
        return ;
    }

    if (!thumbnail_render_param_.IsValid())
    {
        TN_LOG(TN_LOG_INFO) << "No valid shader program";
        return ;
    }

   CheckGLError(glUseProgram(thumbnail_render_param_.shader_program));
   CheckGLError(glUniform1i(thumbnail_render_param_.sample_handle, 0));

   CheckGLError(glBindVertexArray(thumbnail_render_param_.vao_));
   CheckGLError(glActiveTexture(GL_TEXTURE0));
   CheckGLError(glBindTexture(texture_type, input_textures.first));
   CheckGLError(glDrawArrays(GL_TRIANGLES, 0, 6));
   CheckGLError(glBindTexture(texture_type, 0));
   CheckGLError(glBindVertexArray(0));
#else
    // TN_LOG_HZ(TN_LOG_WARNING, 0.01) << "Not support!";
#endif  // TN_PLATFORM_LINUX_TDA4_AARCH64
}

void CameraRenderer::SetInputImages(const std::vector<ImageInput>& input)
{
    for (auto& i : input)
    {
        if (i.format == ImageFormat::kInvalid)
        {
            continue ;
        }
        TextureUtil::Instance()->UpdateTexture(textures_[i.area], i.width, i.height, (unsigned char*)i.data, i.format, i.format);
    }
}

void CameraRenderer::GetLeftTopInComposedCanvas(RenderType type, Position pos, int& left, int& top) const
{
    if (!render_params_.count(type) || !render_params_.at(type).count(pos))
        return;
    switch (type)
    {
    case kFarAvm:
        left = 0;
        top = 0;
        break;
    case kMediumAvm:
        left = 0;
        top = render_params_.at(kMediumAvm).at(kFront).pixel_y_in_composed_canvas;
        break;
    case kNearAvm:
        left = render_params_.at(kNearAvm).at(kLeft).pixel_x_in_composed_canvas;
        top = 0;
        break;
    case kNearAvmVPA:
        left = render_params_.at(kNearAvmVPA).at(kLeft).pixel_x_in_composed_canvas;
        top = 0;
        break;
    case kCustomAvm1:
        left = render_params_.at(kCustomAvm1).at(kLeft).pixel_x_in_composed_canvas;
        top = 0;
        break;
    case kPerspective:
        left = render_params_.at(kPerspective).at(pos).pixel_x_in_composed_canvas;
        top = render_params_.at(kPerspective).at(pos).pixel_y_in_composed_canvas;
        break;
    
    default:
        break;
    }
}

void CameraRenderer::LutToTextureBuff(const RenderParam& render_param, Position lut_key, std::vector<std::vector<GLfloat, StitchingAllocator<GLfloat>>>& dst_texture,
std::vector<GLfloat, StitchingAllocator<GLfloat>>& dst_fusion)
{
    int x_offset = 0;
    int y_offset = 0;
    auto& curr_lut = lut_table_[render_param.type][lut_key];
    GetLutOffset(render_param.type, render_param.position, lut_key, x_offset, y_offset);
    uint32_t* raw_coord = (uint32_t*)&curr_lut.coord_buff[0];

    double lut_pixel_per_grid_x = (double)render_param.clipped_lut_width / render_param.grid_per_row;
    double lut_pixel_per_grid_y = (double)render_param.clipped_lut_height / render_param.grid_per_column;

    // bool debug = (render_param.type == RenderType::kFarAvm && render_param.position != Position::kLeft);

    for (int i = 0; i < render_param.grid_per_column; ++i)
    {
        for (int j = 0; j < render_param.grid_per_row; ++j)
        {
            if (render_param.type != RenderType::kPerspective)
            {
                // 0,0->1,0->1,1->0,1
                int grid_p0_raw = int(i * lut_pixel_per_grid_y + y_offset) * curr_lut.param.width + int(lut_pixel_per_grid_x * j) + x_offset;
                int grid_p1_raw = int(i * lut_pixel_per_grid_y + y_offset) * curr_lut.param.width + int(lut_pixel_per_grid_x * j + (lut_pixel_per_grid_x -1)) + x_offset;
                int grid_p2_raw = int(i * lut_pixel_per_grid_y + (lut_pixel_per_grid_y -1) + y_offset) * curr_lut.param.width + int(lut_pixel_per_grid_x * j + (lut_pixel_per_grid_x -1)) + x_offset;
                int grid_p3_raw = int(i * lut_pixel_per_grid_y + (lut_pixel_per_grid_y -1) + y_offset) * curr_lut.param.width + int(lut_pixel_per_grid_x * j) + x_offset;

                int txt_idx = (i * render_param.grid_per_row + j) * vertex_per_grid_ * 2;

                const bool is_bev = true;
                FillTextureCoord(dst_texture, txt_idx, raw_coord, grid_p0_raw, curr_lut.param, is_bev);
                if (vertex_per_grid_ == 6) {
                    FillTextureCoord(dst_texture, txt_idx + 2, raw_coord, grid_p1_raw, curr_lut.param, is_bev);
                    FillTextureCoord(dst_texture, txt_idx + 4, raw_coord, grid_p2_raw, curr_lut.param, is_bev);
                    FillTextureCoord(dst_texture, txt_idx + 6, raw_coord, grid_p3_raw, curr_lut.param, is_bev);
                    FillTextureCoord(dst_texture, txt_idx + 8, raw_coord, grid_p2_raw, curr_lut.param, is_bev);
                    FillTextureCoord(dst_texture, txt_idx + 10, raw_coord, grid_p0_raw, curr_lut.param, is_bev);
                }

                // if (debug) std::cout << "dst_texture " << j << ", " << i << " -> " << dst_texture[0][txt_idx] << std::endl;

                if (dst_fusion.size() > 0)
                {
                    int wt_idx = (i * render_param.grid_per_row + j) * vertex_per_grid_;

                    dst_fusion[wt_idx] = ReadFloat(raw_coord[grid_p0_raw * BevNumPackIndex::Count + BevNumPackIndex::Coeff]);
                    if (vertex_per_grid_ == 6) {
                        dst_fusion[wt_idx + 1] = ReadFloat(raw_coord[grid_p1_raw * BevNumPackIndex::Count + BevNumPackIndex::Coeff]);
                        dst_fusion[wt_idx + 2] = ReadFloat(raw_coord[grid_p2_raw * BevNumPackIndex::Count + BevNumPackIndex::Coeff]);
                        dst_fusion[wt_idx + 3] = ReadFloat(raw_coord[grid_p3_raw * BevNumPackIndex::Count + BevNumPackIndex::Coeff]);
                        dst_fusion[wt_idx + 4] = ReadFloat(raw_coord[grid_p2_raw * BevNumPackIndex::Count + BevNumPackIndex::Coeff]);
                        dst_fusion[wt_idx + 5] = ReadFloat(raw_coord[grid_p0_raw * BevNumPackIndex::Count + BevNumPackIndex::Coeff]);
                    }

                    // if (debug) std::cout << "dst_fusion " << j << ", " << i << " -> " << dst_fusion[wt_idx] << std::endl;
                }
            }
            else
            {
                int grid_p0_raw = int(i * lut_pixel_per_grid_y) * curr_lut.param.width + int(lut_pixel_per_grid_x * j);
                int grid_p1_raw = int(i * lut_pixel_per_grid_y) * curr_lut.param.width + int(lut_pixel_per_grid_x * j + (lut_pixel_per_grid_x -1));
                int grid_p2_raw = int(i * lut_pixel_per_grid_y + (lut_pixel_per_grid_y -1)) * curr_lut.param.width + int(lut_pixel_per_grid_x * j + (lut_pixel_per_grid_x -1));
                int grid_p3_raw = int(i * lut_pixel_per_grid_y + (lut_pixel_per_grid_y -1)) * curr_lut.param.width + int(lut_pixel_per_grid_x * j);

                int txt_idx = (i * render_param.grid_per_row + j) * vertex_per_grid_ * 2;

                const bool is_bev = false;
                FillTextureCoord(dst_texture, txt_idx, raw_coord, grid_p0_raw, curr_lut.param, is_bev);
                if (vertex_per_grid_ == 6) {
                    FillTextureCoord(dst_texture, txt_idx + 2, raw_coord, grid_p1_raw, curr_lut.param, is_bev);
                    FillTextureCoord(dst_texture, txt_idx + 4, raw_coord, grid_p2_raw, curr_lut.param, is_bev);
                    FillTextureCoord(dst_texture, txt_idx + 6, raw_coord, grid_p3_raw, curr_lut.param, is_bev);
                    FillTextureCoord(dst_texture, txt_idx + 8, raw_coord, grid_p2_raw, curr_lut.param, is_bev);
                    FillTextureCoord(dst_texture, txt_idx + 10, raw_coord, grid_p0_raw, curr_lut.param, is_bev);
                }
            }
        }
    }
}

template<typename T> void CameraRenderer::SetGlBuffer(GLuint target, GLuint id, const std::vector<T, StitchingAllocator<T>>& data)
{
   CheckGLError(glBindBuffer(target, id));
   CheckGLError(glBufferData(target, sizeof(T) * data.size(), &data[0], GL_STATIC_DRAW));
}

void CameraRenderer::UpdateVbo(std::map<Position, RenderParam>& param_table)
{
    for(auto& p : param_table)
    {
        auto& vertex_buff = vertex_buff_[p.second.type][p.first];
        auto& index_buff = index_buff_[p.second.type][p.first];
        auto& texture_buff = texture_buff_[p.second.type][p.first];
        auto& fusion_weight_buff = fusion_weight_buff_[p.second.type][p.first];

        SetGlBuffer(GL_ARRAY_BUFFER, p.second.vbo_table[VboType::kVertex], vertex_buff);
        // SetGlBuffer(GL_ELEMENT_ARRAY_BUFFER, p.second.vbo_table[VboType::kIndex], index_buff);
        SetGlBuffer(GL_ARRAY_BUFFER, p.second.vbo_table[VboType::kTexture0], texture_buff[0][0]);

        if (texture_buff.size() > 1 && texture_buff[1].size() > 0)
        {
            SetGlBuffer(GL_ARRAY_BUFFER, p.second.vbo_table[VboType::kTexture1], texture_buff[1][0]);
            SetGlBuffer(GL_ARRAY_BUFFER, p.second.vbo_table[VboType::kFusionWeight], fusion_weight_buff[0]);
        }
    }
}

void CameraRenderer::GenVbo()
{
    for (auto &i : render_params_)
    {
        for (auto &j : i.second)
        {
            if (overlapped_area_.end() != overlapped_area_.find(j.first))
            {
                if (i.first == RenderType::kPerspective)
                    continue;
               CheckGLError(glGenBuffers(1, &j.second.vbo_table[VboType::kTexture1]));
               CheckGLError(glGenBuffers(1, &j.second.vbo_table[VboType::kFusionWeight]));
            }
           CheckGLError(glGenBuffers(1, &j.second.vbo_table[VboType::kVertex]));
           CheckGLError(glGenBuffers(1, &j.second.vbo_table[VboType::kTexture0]));
           CheckGLError(glGenBuffers(1, &j.second.vbo_table[VboType::kIndex]));
        }
    }

#if defined(TN_PLATFORM_LINUX_TDA4_AARCH64)
    do
    {
        if (!ConfigManager::Instance()->GenerateSurroundThumnails())
        {
            break ;
        }

        // float thumb_vetices[] = {
        //     // positions        // tex-coords
        //     -1.0, -1.0,         0.0, 0.0,
        //     -1.0, 1.0,          0.0, 1.0,
        //     1.0, 1.0,           1.0, 1.0,

        //     -1.0, -1.0,         0.0, 0.0,
        //     1.0, 1.0,           1.0, 1.0,
        //     1.0, -1.0,          1.0, 0.0
        // };
        float thumb_vetices[] = {
            // positions   // texCoords
            -1.0f,  1.0f,  0.0f, 0.0f,
            -1.0f, -1.0f,  0.0f, 1.0f,
            1.0f, -1.0f,  1.0f, 1.0f,

            -1.0f,  1.0f,  0.0f, 0.0f,
            1.0f, -1.0f,  1.0f, 1.0f,
            1.0f,  1.0f,  1.0f, 0.0f
        };

        GLuint thumb_vbo;
       CheckGLError(glGenVertexArrays(1, &thumbnail_render_param_.vao_));
       CheckGLError(glGenBuffers(1, &thumb_vbo));

       CheckGLError(glBindVertexArray(thumbnail_render_param_.vao_));

       CheckGLError(glBindBuffer(GL_ARRAY_BUFFER, thumb_vbo));
       CheckGLError(glBufferData(GL_ARRAY_BUFFER, sizeof(thumb_vetices), &thumb_vetices, GL_STATIC_DRAW));
       CheckGLError(glEnableVertexAttribArray(0));
       CheckGLError(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0));
       CheckGLError(glEnableVertexAttribArray(1));
       CheckGLError(glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float))));
       CheckGLError(glBindVertexArray(0));

    } while(false);
#endif  // TN_PLATFORM_LINUX_TDA4_AARCH64
}

void CameraRenderer::InitVbo()
{
    GenVbo();
    for (auto& type : render_params_)
    {
        if (ConfigManager::Instance()->RenderEnabled(GetOutputType(type.first)))
            UpdateVbo(type.second);
    }
}

void CameraRenderer::GetLutOffset(RenderType type, Position render_pos, Position lut_pos, int& x_offset, int& y_offset)
{
    x_offset = 0;
    y_offset = 0;
    if (!IsOverlappedArea(render_pos))
        return;

    switch (render_pos)
    {
    case Position::kFrontLeft:
        break;
    case Position::kFrontRight:
            if (Position::kFront == lut_pos)
                x_offset = lut_table_[type][lut_pos].param.width - lut_table_[type][Position::kRight].param.width;
        break;
    case Position::kRearLeft:
            if (Position::kLeft == lut_pos)
                y_offset = lut_table_[type][lut_pos].param.height - lut_table_[type][Position::kRear].param.height;
        break;
    case Position::kRearRight:
            if (Position::kRear == lut_pos)
                x_offset = lut_table_[type][lut_pos].param.width - lut_table_[type][Position::kRight].param.width;
            else
                y_offset = lut_table_[type][lut_pos].param.height - lut_table_[type][Position::kRear].param.height;
        break;
    default:
        break;
    }
}

void CameraRenderer::InitRenderBoundries()
{
    opengl_left_ = opengl_left_ + 1.0 / ConfigManager::Instance()->CanvasWidth();
    opengl_top_ = opengl_top_ - 1.0 / ConfigManager::Instance()->CanvasHeight();
    InitGridParam();
    auto update_avm_boundry = [&](RenderType rtype, std::map<Position, RenderParam>& param_table) {
        float max_w = opengl_max_width_;
        float max_h = opengl_max_height_;
        max_w *= (float)ConfigManager::Instance()->OutputImageWidth(GetOutputType(rtype)) / ConfigManager::Instance()->CanvasWidth();
        max_h *= (float)ConfigManager::Instance()->OutputImageHeight(GetOutputType(rtype)) / ConfigManager::Instance()->CanvasHeight();
        param_table[Position::kFront].opengl_render_x = opengl_left_;
        param_table[Position::kFront].opengl_render_y = opengl_top_;
        param_table[Position::kFront].opengl_render_width = max_w;
        param_table[Position::kFront].opengl_render_height = max_h * lut_table_[rtype][Position::kFront].param.height / lut_table_[rtype][Position::kLeft].param.height;
        param_table[Position::kRear].CopyBoundry(param_table[Position::kFront]);
        param_table[Position::kRear].opengl_render_height = max_h * lut_table_[rtype][Position::kRear].param.height / lut_table_[rtype][Position::kLeft].param.height;
        param_table[Position::kRear].opengl_render_y -= max_h - param_table[Position::kRear].opengl_render_height;
        param_table[Position::kLeft].CopyBoundry(param_table[Position::kFront]);
        param_table[Position::kLeft].opengl_render_width = max_w * lut_table_[rtype][Position::kLeft].param.width / lut_table_[rtype][Position::kFront].param.width;
        param_table[Position::kLeft].opengl_render_height = max_h;
        param_table[Position::kRight].CopyBoundry(param_table[Position::kLeft]);
        param_table[Position::kRight].opengl_render_x += max_w - param_table[Position::kRight].opengl_render_width;

        param_table[Position::kFrontLeft].CopyBoundry(param_table[Position::kFront]);
        param_table[Position::kFrontLeft].opengl_render_width = param_table[Position::kLeft].opengl_render_width;
        param_table[Position::kFrontRight].CopyBoundry(param_table[Position::kRight]);
        param_table[Position::kFrontRight].opengl_render_height = param_table[Position::kFront].opengl_render_height;
        param_table[Position::kRearLeft].CopyBoundry(param_table[Position::kRear]);
        param_table[Position::kRearLeft].opengl_render_width = param_table[Position::kLeft].opengl_render_width;
        param_table[Position::kRearRight].CopyBoundry(param_table[Position::kRear]);
        param_table[Position::kRearRight].opengl_render_x = param_table[Position::kRight].opengl_render_x;
        param_table[Position::kRearRight].opengl_render_width = param_table[Position::kRight].opengl_render_width;
    };
    auto update_perspective_boundry = [&](std::map<Position, RenderParam>& param_table) {
        float max_w = opengl_max_width_;
        float max_h = opengl_max_height_;
        max_w *= (float)ConfigManager::Instance()->OutputImageWidth(OutputType::kPerspectiveAll) / ConfigManager::Instance()->CanvasWidth();
        max_h *= (float)ConfigManager::Instance()->OutputImageHeight(OutputType::kPerspectiveAll) / ConfigManager::Instance()->CanvasHeight();

        param_table[Position::kFront].opengl_render_x = opengl_left_;
        param_table[Position::kFront].opengl_render_y = opengl_top_;
        param_table[Position::kFront].opengl_render_width = max_w;
        param_table[Position::kFront].opengl_render_height = max_h;
        param_table[Position::kRear].CopyBoundry(param_table[Position::kFront]);
        param_table[Position::kLeft].CopyBoundry(param_table[Position::kFront]);
        param_table[Position::kRight].CopyBoundry(param_table[Position::kFront]);
    };
    for (auto& type : render_params_)
    {
        if (RenderType::kFarAvm == type.first || RenderType::kMediumAvm == type.first || RenderType::kNearAvm == type.first
            || RenderType::kNearAvmVPA == type.first || RenderType::kCustomAvm1 == type.first)
            update_avm_boundry(type.first, type.second);
        else
            update_perspective_boundry(type.second);
    }
}

void CameraRenderer::MapRenderBoundriesToComposedCanvas(std::map<RenderType, std::map<Position, RenderParam>>& data)
{
    /*
    ___________________________________
    |       |near |  persp  |  persp  |
    | far   |(apa |  front  |   left  |
    |_______|vpa) |_________|_________|
    |       |_____|  persp  |  persp  |
    | medium|     |   rear  |  right  |
    |_______|unuse|_________|_________|
    */
    double near_around_hoffset(0.0), medium_around_voffset(0.0), perspect_front_left(0.0), perspect_rear_top(0.0), perspect_left_left(0.0);
    int far_w = ConfigManager::Instance()->OutputImageWidth(GetOutputType(RenderType::kFarAvm));
    int far_h = ConfigManager::Instance()->OutputImageHeight(GetOutputType(RenderType::kFarAvm));
    int near_w = std::max(ConfigManager::Instance()->OutputImageWidth(GetOutputType(RenderType::kNearAvm)),
        ConfigManager::Instance()->OutputImageWidth(GetOutputType(RenderType::kNearAvmVPA)));
    int persp_w = ConfigManager::Instance()->OutputImageWidth(GetOutputType(RenderType::kPerspective));
    int persp_h = ConfigManager::Instance()->OutputImageHeight(GetOutputType(RenderType::kPerspective));
    auto canvas_w = ConfigManager::Instance()->CanvasWidth();
    if (canvas_w == 0)
        canvas_w = 1;
    auto canvas_h = ConfigManager::Instance()->CanvasHeight();
    if (canvas_h == 0)
        canvas_h = 1;
    near_around_hoffset = double(far_w) * opengl_max_width_ / canvas_w;
    medium_around_voffset = double(far_h) * opengl_max_height_ / canvas_h;
    perspect_front_left = -1.0 + double(far_w + near_w) * opengl_max_width_ / canvas_w;
    perspect_left_left = -1.0 + double(far_w + near_w + persp_w) * opengl_max_width_ / canvas_w;
    perspect_rear_top = -1.0 + double(persp_h) * opengl_max_height_ / canvas_h;
    data[RenderType::kNearAvm][Position::kLeft].pixel_x_in_composed_canvas = far_w;
    data[RenderType::kNearAvmVPA][Position::kLeft].pixel_x_in_composed_canvas = far_w;
    for (auto& p : data[RenderType::kMediumAvm])
    {
        p.second.opengl_render_y -= medium_around_voffset;
        p.second.pixel_y_in_composed_canvas = far_h;
    }
    for (auto& p : data[RenderType::kNearAvm])
        p.second.opengl_render_x += near_around_hoffset;
    for (auto& p : data[RenderType::kNearAvmVPA])
        p.second.opengl_render_x += near_around_hoffset;
    for (auto& p : data[RenderType::kCustomAvm1])
        p.second.opengl_render_x += near_around_hoffset;
    for (auto& p : data[RenderType::kPerspective])
    {
        switch (p.first)
        {
        case Position::kFront:
            p.second.opengl_render_x = perspect_front_left;
            p.second.pixel_x_in_composed_canvas = far_w + near_w;
            break;
        case Position::kRear:
            p.second.opengl_render_x = perspect_front_left;
            p.second.opengl_render_y = perspect_rear_top;
            p.second.pixel_x_in_composed_canvas = far_w + near_w;
            p.second.pixel_y_in_composed_canvas = persp_h;
            break;
        case Position::kLeft:
            p.second.opengl_render_x = perspect_left_left;
            p.second.pixel_x_in_composed_canvas = far_w + near_w + persp_w;
            break;
        case Position::kRight:
            p.second.opengl_render_x = perspect_left_left;
            p.second.opengl_render_y = perspect_rear_top;
            p.second.pixel_x_in_composed_canvas = far_w + near_w + persp_w;
            p.second.pixel_y_in_composed_canvas = persp_h;
            break;
        
        default:
            break;
        }
    }
}

void CameraRenderer::InitShaders()
{
    auto set_program = [&](GLuint program, bool for_overlapped) {
        for (auto& r : render_params_)
        {
            for (auto& m : render_params_[r.first])
            {
                if (for_overlapped == IsOverlappedArea(m.second.position))
                    m.second.shader_program = program;
            }
        }
    };

    if (ConfigManager::Instance())
    {
        ImageFormat input_image_format = ConfigManager::Instance()->InputImageFormat();
        bool is_egl_input = ConfigManager::Instance()->EglimageInput();

        std::string unmerge_postfix = "unmerge2d_rgb.frag";
        std::string merge_postfix = "merge2d_rgb.frag";


        if (is_egl_input)
        {
            if (ImageFormat::kNv12 == input_image_format)   
            {
                unmerge_postfix = "unmerge2d_nv12_egl.frag";
                merge_postfix = "merge2d_nv12_egl.frag";
            }
        }
        else
        {
            if (ImageFormat::kNv12 == input_image_format)   
            {
                unmerge_postfix = "unmerge2d_nv12.frag";
                merge_postfix = "merge2d_nv12.frag";
            }
        }

        std::string shader_path = ConfigManager::Instance()->ShaderPath();
        GLuint program;
        if (0 != ShaderUtil::LoadShader(shader_path + "/unmerge2d.vert", shader_path + unmerge_postfix, program))
            return;
        shader_programs_[program][ShaderAttribute::kPosition] =CheckGLError(glGetAttribLocation(program, "position"));
        shader_programs_[program][ShaderAttribute::kTextureCoord0] =CheckGLError(glGetAttribLocation(program, "texture_coord_vert_0"));
        shader_programs_[program][ShaderAttribute::kTextureSampler0] =CheckGLError(glGetUniformLocation(program, "sampler_0"));
        shader_programs_[program][ShaderAttribute::kTextureSampler00] =CheckGLError(glGetUniformLocation(program, "sampler_00"));
        shader_programs_[program][ShaderAttribute::kDx] =CheckGLError(glGetUniformLocation(program, "dx"));
        shader_programs_[program][ShaderAttribute::kDy] =CheckGLError(glGetUniformLocation(program, "dy"));
        set_program(program, false);

        if (0 != ShaderUtil::LoadShader(shader_path + "/merge2d.vert", shader_path + merge_postfix, program))
            return;
        shader_programs_[program][ShaderAttribute::kPosition] =CheckGLError(glGetAttribLocation(program, "position"));
        shader_programs_[program][ShaderAttribute::kTextureCoord0] =CheckGLError(glGetAttribLocation(program, "texture_coord_vert_0"));
        shader_programs_[program][ShaderAttribute::kTextureCoord1] =CheckGLError(glGetAttribLocation(program, "texture_coord_vert_1"));
        shader_programs_[program][ShaderAttribute::kTextureSampler0] =CheckGLError(glGetUniformLocation(program, "sampler_0"));
        shader_programs_[program][ShaderAttribute::kTextureSampler00] =CheckGLError(glGetUniformLocation(program, "sampler_00"));
        shader_programs_[program][ShaderAttribute::kTextureSampler1] =CheckGLError(glGetUniformLocation(program, "sampler_1"));
        shader_programs_[program][ShaderAttribute::kTextureSampler11] =CheckGLError(glGetUniformLocation(program, "sampler_11"));
        shader_programs_[program][ShaderAttribute::kDx] =CheckGLError(glGetUniformLocation(program, "dx"));
        shader_programs_[program][ShaderAttribute::kDy] =CheckGLError(glGetUniformLocation(program, "dy"));
        shader_programs_[program][ShaderAttribute::kWeight] =CheckGLError(glGetAttribLocation(program, "weight_vert"));
        set_program(program, true);

        if (0 != ShaderUtil::LoadShader(shader_path + "/texture_thumbnail.vert", shader_path + "/texture_thumbnail.frag", program))
        {
            TN_LOG(TN_LOG_ERROR) << "Failed to load shader texture_thumbnail.vert and texture_thumbnail.frag in " << shader_path; 
            return;
        }

#if defined(TN_PLATFORM_LINUX_TDA4_AARCH64)
        if (ConfigManager::Instance()->GenerateSurroundThumnails())
        {
            thumbnail_render_param_.shader_program = program;
            thumbnail_render_param_.sample_handle =CheckGLError(glGetUniformLocation(program, "sTexture"));
        }
#endif  // TN_PLATFORM_LINUX_TDA4_AARCH64
    }
    else
    {
        TN_LOG(TN_LOG_ERROR) << "Memory allocated failed.";
    }
 
}

void CameraRenderer::Triangulate(GLfloat* addr, GLfloat x, GLfloat y, GLfloat x_jump, GLfloat y_jump, int dimension)
{
    if (dimension == 3)
        addr[2] = addr[5] = addr[8] = addr[11] = addr[14] = addr[17] = 0.0;
    addr[0] = x;
    addr[1] = y;
    addr[dimension * 1] = x + x_jump;
    addr[dimension * 1 + 1] = y;
    addr[dimension * 2] = x + x_jump;
    addr[dimension * 2 + 1] = y - y_jump;
    addr[dimension * 3] = x;
    addr[dimension * 3 + 1] = y - y_jump;
    addr[dimension * 4] = x + x_jump;
    addr[dimension * 4 + 1] = y - y_jump;
    addr[dimension * 5] = x;
    addr[dimension * 5 + 1] = y;
}

void CameraRenderer::Tessellate(const RenderParam& param, std::vector<GLfloat, StitchingAllocator<GLfloat>>& vertex_buff)
{
    GLfloat x_jump = param.opengl_render_width / param.grid_per_row;
    GLfloat y_jump = param.opengl_render_height / param.grid_per_column;
    for (int i = 0; i < param.grid_per_column; ++i)
    {
        GLfloat ref_y = param.opengl_render_y - param.opengl_render_height * i / param.grid_per_column;
        for (int j = 0; j < param.grid_per_row; ++j)
        {
            GLfloat ref_x = param.opengl_render_x + param.opengl_render_width * j / param.grid_per_row;
            vertex_buff[(i * param.grid_per_row + j) * vertex_dimension_] = ref_x;
            vertex_buff[(i * param.grid_per_row + j) * vertex_dimension_ + 1] = ref_y;
            //Triangulate(&vertex_buff[(i * param.grid_per_row + j) * vertex_per_grid_ * vertex_dimension_], ref_x, ref_y, x_jump, y_jump, vertex_dimension_);
        }
    }
}

void CameraRenderer::InitRenderBuffers()
{
    for (auto& type : render_params_)
    {
#ifdef TN_PLATFORM_LINUX_TDA4_FAW_AARCH64
        auto output_type = GetOutputType(type.first);
        if (!ConfigManager::Instance()->RenderEnabled(output_type)) continue;
#endif  // TN_PLATFORM_LINUX_TDA4_FAW_AARCH64
        for(auto& param : type.second)
        {
            int vertex_count = param.second.grid_per_row * param.second.grid_per_column;
            std::vector<GLfloat, StitchingAllocator<GLfloat>> vertex_buff(vertex_count * vertex_dimension_);
            std::vector<GLuint, StitchingAllocator<GLuint>> index_buff(vertex_count);
            std::vector<std::vector<GLfloat, StitchingAllocator<GLfloat>>> fusion_weight_buff;
            for (int i = 0; i < index_buff.size(); ++i)
                index_buff[i] = i;
            Tessellate(param.second, vertex_buff);
            for (int i = 0; i < param.second.lut_keys.size(); ++i)
            {
                std::vector<std::vector<GLfloat, StitchingAllocator<GLfloat>>> texture_buff;
                auto& lut_key = param.second.lut_keys[i];
                for (int j = 0; j < 1; ++j)
                {
                    texture_buff.emplace_back(std::vector<GLfloat, StitchingAllocator<GLfloat>>(vertex_count * 2));
                }
                if (IsOverlappedArea(param.first))
                    fusion_weight_buff.emplace_back(std::vector<GLfloat, StitchingAllocator<GLfloat>>(vertex_count));
                std::vector<GLfloat, StitchingAllocator<GLfloat>> weight;
                LutToTextureBuff(param.second, lut_key, texture_buff, fusion_weight_buff.size() > i ? fusion_weight_buff[i] : weight);
                if (texture_buff_[type.first].size() < param.first + 1)
                    texture_buff_[type.first].resize(param.first + 1);
                texture_buff_[type.first][param.first].emplace_back(std::move(texture_buff));
            }
            if (vertex_buff_[type.first].size() < param.first + 1)
                vertex_buff_[type.first].resize(param.first + 1);
            vertex_buff_[type.first][param.first] = std::move(vertex_buff);
            if (index_buff_[type.first].size() < param.first + 1)
                index_buff_[type.first].resize(param.first + 1);
            index_buff_[type.first][param.first] = std::move(index_buff);
            if (fusion_weight_buff_[type.first].size() < param.first + 1)
                fusion_weight_buff_[type.first].resize(param.first + 1);
            fusion_weight_buff_[type.first][param.first] = std::move(fusion_weight_buff);
        }
    }
}

void CameraRenderer::Uninit()
{
    RecollectMemory();
    for (auto& i : render_params_)
    {
        for (auto& j : i.second)
        {
            for (auto& k : j.second.vbo_table)
            {
               CheckGLError(glDeleteBuffers(1, &k.second));
            }
            j.second.vbo_table.clear();
        }
        i.second.clear();
    }
    render_params_.clear();

    for (auto& i : textures_)
    {
       CheckGLError(glDeleteBuffers(1, &i.second));
    }
    textures_.clear();

#if defined(TN_PLATFORM_LINUX_TDA4_AARCH64)
    if (ConfigManager::Instance()->GenerateSurroundThumnails())
    {
       CheckGLError(glDeleteBuffers(1, &thumbnail_render_param_.vao_));
       CheckGLError(glDeleteProgram(thumbnail_render_param_.shader_program));
        thumbnail_render_param_.vao_ = 0;
    }
#endif // TN_PLATFORM_LINUX_TDA4_AARCH64

    for (auto& i : shader_programs_)
    {
        for (auto& j : i.second)
        {
           CheckGLError(glDisableVertexAttribArray(j.second));
        }
        i.second.clear();
       CheckGLError(glDeleteProgram(i.first));
    }
    shader_programs_.clear();
    position_list_.clear();
    overlapped_area_.clear();
    lut_table_.clear();
}

void CameraRenderer::LoadLut(const LutParam& param, std::vector<unsigned char>& coord)
{
    std::ifstream ifs(param.coord_path, std::ios::binary);
    if (!ifs.is_open()) {
        TN_LOG(TN_LOG_ERROR) << " LoadLut fail";
        // std::cout << "LoadLut fail " << param.coord_path << std::endl;
        return;
    }
    coord.assign(std::istreambuf_iterator<char>(ifs), {});
}

void CameraRenderer::InitLutParam()
{
    for (auto &type : render_params_)
    {
        lut_table_[type.first].resize(4);
        lut_table_[type.first][Position::kFront] = LutRenderData(ConfigManager::Instance()->GetLutParam(type.first, Position::kFront));
        lut_table_[type.first][Position::kRear] = LutRenderData(ConfigManager::Instance()->GetLutParam(type.first, Position::kRear));
        lut_table_[type.first][Position::kLeft] = LutRenderData(ConfigManager::Instance()->GetLutParam(type.first, Position::kLeft));
        lut_table_[type.first][Position::kRight] = LutRenderData(ConfigManager::Instance()->GetLutParam(type.first, Position::kRight));
    }

    for (auto &i : lut_table_)
    {
#ifdef TN_PLATFORM_LINUX_TDA4_FAW_AARCH64
        auto output_type = GetOutputType(i.first);
        if (!ConfigManager::Instance()->RenderEnabled(output_type))
            continue;
#endif  // TN_PLATFORM_LINUX_TDA4_FAW_AARCH64
        for (auto& j : i.second)
        {
            LoadLut(j.param, j.coord_buff);
        }
    }
    for (auto& i : render_params_)
    {
        for (auto& j : i.second)
        {
            if (!IsOverlappedArea(j.first))
            {
                j.second.lut_keys.push_back(j.first);
                j.second.clipped_lut_width = lut_table_[i.first][j.first].param.width;
                j.second.clipped_lut_height = lut_table_[i.first][j.first].param.height;
                continue;
            }
            switch (j.first)
            {
            case Position::kFrontLeft:
                j.second.lut_keys.push_back(Position::kFront);
                j.second.lut_keys.push_back(Position::kLeft);
                j.second.clipped_lut_width = lut_table_[i.first][Position::kLeft].param.width;
                j.second.clipped_lut_height = lut_table_[i.first][Position::kFront].param.height;
                break;
            case Position::kFrontRight:
                j.second.lut_keys.push_back(Position::kFront);
                j.second.lut_keys.push_back(Position::kRight);
                j.second.clipped_lut_width = lut_table_[i.first][Position::kRight].param.width;
                j.second.clipped_lut_height = lut_table_[i.first][Position::kFront].param.height;
                break;
            case Position::kRearLeft:
                j.second.lut_keys.push_back(Position::kRear);
                j.second.lut_keys.push_back(Position::kLeft);
                j.second.clipped_lut_width = lut_table_[i.first][Position::kLeft].param.width;
                j.second.clipped_lut_height = lut_table_[i.first][Position::kRear].param.height;
                break;
            case Position::kRearRight:
                j.second.lut_keys.push_back(Position::kRear);
                j.second.lut_keys.push_back(Position::kRight);
                j.second.clipped_lut_width = lut_table_[i.first][Position::kRight].param.width;
                j.second.clipped_lut_height = lut_table_[i.first][Position::kRear].param.height;
                break;
            
            default:
                break;
            }
        }
    }
}

void CameraRenderer::InitParam()
{
    auto insert_pos = [&](RenderType type, std::map<Position, RenderParam>& table) {
        for (int i = 0; i < position_list_.size(); ++i)
        {
            if (type == RenderType::kPerspective && overlapped_area_.end() != overlapped_area_.find(position_list_[i]))
                continue;
            table[position_list_[i]].type = type;
            table[position_list_[i]].position = position_list_[i];
        }
    };
    insert_pos(RenderType::kFarAvm, render_params_[RenderType::kFarAvm]);
    insert_pos(RenderType::kMediumAvm, render_params_[RenderType::kMediumAvm]);
    insert_pos(RenderType::kNearAvm, render_params_[RenderType::kNearAvm]);
    insert_pos(RenderType::kNearAvmVPA, render_params_[RenderType::kNearAvmVPA]);
    insert_pos(RenderType::kCustomAvm1, render_params_[RenderType::kCustomAvm1]);
    insert_pos(RenderType::kPerspective, render_params_[RenderType::kPerspective]);
    InitLutParam();
    InitRenderBoundries();
    if (ConfigManager::Instance()->ComposedCanvas())
        MapRenderBoundriesToComposedCanvas(render_params_);
}

void CameraRenderer::InitGridParam()
{
    for (auto &i : render_params_)
    {
        for (auto &j : i.second)
        {
            auto p = ConfigManager::Instance()->GetRenderParam(i.first, j.first);
            j.second.grid_per_column = p.grid_per_column;
            j.second.grid_per_row = p.grid_per_row;
        }
    }
}

void CameraRenderer::InitTexture()
{
    textures_[Position::kFront] = TextureUtil::Instance()->GenerateTexture(GL_TEXTURE_2D);
    textures_[Position::kRear] = TextureUtil::Instance()->GenerateTexture(GL_TEXTURE_2D);
    textures_[Position::kLeft] = TextureUtil::Instance()->GenerateTexture(GL_TEXTURE_2D);
    textures_[Position::kRight] = TextureUtil::Instance()->GenerateTexture(GL_TEXTURE_2D);
}

void CameraRenderer::RecollectMemory()
{
    texture_buff_.clear();
    fusion_weight_buff_.clear();
    lut_table_.clear();
    index_buff_.clear();
    // mallopt(M_TRIM_THRESHOLD, 10000);
    // malloc_trim(1024);
    MemoryPool::Instance()->releaseAllMemToOs();
}

void CameraRenderer::InitMassiveData()
{
    position_list_ = std::move(std::vector<Position>({Position::kFront, Position::kRear, Position::kLeft, Position::kRight, Position::kFrontLeft,
    Position::kFrontRight, Position::kRearLeft, Position::kRearRight}));
    overlapped_area_ = std::move(std::set<Position>({Position::kFrontLeft, Position::kFrontRight, Position::kRearLeft, Position::kRearRight}));
    InitParam();
    InitRenderBuffers();
}

void CameraRenderer::InitGlRendering()
{
    InitTexture();
    InitShaders();
    InitVbo();
    RecollectMemory();
}

void CameraRenderer::Init()
{
    position_list_ = std::move(std::vector<Position>({Position::kFront, Position::kRear, Position::kLeft, Position::kRight, Position::kFrontLeft,
    Position::kFrontRight, Position::kRearLeft, Position::kRearRight}));
    overlapped_area_ = std::move(std::set<Position>({Position::kFrontLeft, Position::kFrontRight, Position::kRearLeft, Position::kRearRight}));
    InitParam();
    InitRenderBuffers();
    InitTexture();
    InitShaders();
    InitVbo();
    RecollectMemory();
}

} // namespace stitching {
} // namespace tn {
