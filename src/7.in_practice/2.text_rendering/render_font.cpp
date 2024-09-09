#include "render_font.h"

#include "ft2build.h"
#include "freetype/ftoutln.h"
#include FT_FREETYPE_H


#define GL_GLEXT_PROTOTYPES


#ifndef debug_font

#include "GLES3/gl3.h"
#include "GLES3/gl31.h"
#include "tn_logger.h"
#include "config_manager.h"
#include "shader_util.h"

#endif

namespace tn
{
namespace stitching
{

void FontProperty::Print()
{
    TN_LOG(TN_LOG_ERROR) << "show_width:" << show_width << ", show_height:" << show_height << ", font_size:" << font_size 
        << " max_font_num:" << max_font_num << ", max_line_width:" << max_line_width << ", line_space:" << line_space
        << " font:" << font << ", shadow:" << shadow << " font_weight:" << font_weight << " horizontal_center:" << horizontal_center
        << " vertical_center:" << vertical_center << " new_font_render:" << new_font_render << " debug:" << debug
        ;
}
    RenderFont::RenderFont(const std::wstring &content, const float &x, const float &y, const FontProperty &font_property)
        : init_done_(false), shader_program_(0), ft_(nullptr), face_(nullptr), is_active_(false), 
          content_(content), x_(x), y_(y), font_property_(font_property)
    {
        font_loaded_ = false;
        content_changed_ = !content_.empty();
        
        if(font_property_.font.empty())
        {
            font_property_.font = ConfigManager::Instance()->GetDefautFont();
            
        }
        font_property_.Print();
    }
void RenderFont::SetFontProperty(const FontProperty &font_peroperty) { font_property_ = font_peroperty; 
        TN_LOG(TN_LOG_ERROR) << "======================BEGIN SetFontProperty======================";
        font_property_.Print();
        TN_LOG(TN_LOG_ERROR) << "======================END SetFontProperty======================";
        }
    void RenderFont::Init()
    {
 
        if (init_done_)
            return;
        std::string shader_path = ConfigManager::Instance()->ShaderPath();
        if(tn::stitching::ShaderUtil::LoadShader(shader_path + "font.vert", shader_path + "font.frag", shader_program_) != 0)
		{
            TN_LOG(TN_LOG_ERROR) << "failed to load shader." << std::endl;
            return; 
        }
        // glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(2650), 0.0f, static_cast<float>(1392));
        // glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(2650), static_cast<float>(1392), 0.0f, -1.0f, 1.0f);

       CheckGLError(glUseProgram(shader_program_));
        //CheckGLError(glUniformMatrix4fv(glGetUniformLocation(shader_program_, "projection"), 1, GL_FALSE, glm::value_ptr(projection))); 
       CheckGLError(glGenVertexArrays(1, &vao_));
       CheckGLError(glGenBuffers(1, &vbo_));
       CheckGLError(glBindVertexArray(vao_));
       CheckGLError(glBindBuffer(GL_ARRAY_BUFFER, vbo_));
       CheckGLError(glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW));

       CheckGLError(glEnableVertexAttribArray(0));
       CheckGLError(glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0));
       CheckGLError(glBindBuffer(GL_ARRAY_BUFFER, 0));
       CheckGLError(glBindVertexArray(0));        
         
        for (int i = 0; i < font_property_.max_font_num; ++i)
        {
            unsigned int texture;
           CheckGLError(glGenTextures(1, &texture));
            texture_.push_back(texture);
        } 
        TN_LOG(TN_LOG_INFO) << "INIT FINISHED" << std::endl;
    } 

    void RenderFont::LoadText(const std::wstring &text) 
    { 
        if (!font_loaded_)
        {            
            if (FT_Init_FreeType(&ft_))
                TN_LOG(TN_LOG_INFO) << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;

            if (FT_New_Face(ft_, font_property_.font.c_str(), 0, &face_))
                TN_LOG(TN_LOG_INFO) << "ERROR::FREETYPE: Failed to load font" << std::endl;

            FT_Set_Pixel_Sizes(face_, 0, font_property_.font_size); 

            font_loaded_ = true;
        }

       CheckGLError(glBindVertexArray(vao_));
       CheckGLError(glBindBuffer(GL_ARRAY_BUFFER, vbo_));
        //CheckGLError(glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW));

       CheckGLError(glEnableVertexAttribArray(0));
       CheckGLError(glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0));
        
       CheckGLError(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));

        std::vector<unsigned int> available_texture_indexes;
        available_texture_indexes.reserve(font_property_.max_font_num);

        characters_.clear();        

        text_index_ = 0;
        for (auto c : text)
        {
            if (text_index_ >= font_property_.max_font_num)
            {
                break ;
            }

            if (characters_.count(c) > 0)
            {
                continue;
            }

            if (FT_Load_Char(face_, c, FT_LOAD_DEFAULT | FT_LOAD_NO_BITMAP))
            {
                TN_LOG(TN_LOG_INFO) << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
                continue;
            } 
  
            //FT_Outline_Embolden(&face_->glyph->outline, font_property_.font_weight);
            FT_Render_Glyph(face_->glyph, FT_RENDER_MODE_NORMAL);
            // unsigned int texture;
            //CheckGLError(glGenTextures(1, &texture));
 
           CheckGLError(glBindTexture(GL_TEXTURE_2D, texture_[text_index_]));
            glTexImage2D(GL_TEXTURE_2D, 0,
                         GL_RED, face_->glyph->bitmap.width,
                         face_->glyph->bitmap.rows, 0,
                         GL_RED, GL_UNSIGNED_BYTE,
                         face_->glyph->bitmap.buffer);
 
 
           CheckGLError(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
           CheckGLError(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
           CheckGLError(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
           CheckGLError(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
 
            FT_BBox_ bbox; 
            FT_Glyph glyph;
            FT_Get_Glyph(face_->glyph, &glyph);
            FT_Glyph_Get_CBox(glyph, FT_GLYPH_BBOX_TRUNCATE, &bbox);
            Character character = {texture_[text_index_],
                                   glm::ivec2(face_->glyph->bitmap.width, face_->glyph->bitmap.rows),
                                   glm::ivec2(face_->glyph->bitmap_left, face_->glyph->bitmap_top),
                                   static_cast<unsigned int>(face_->glyph->advance.x),
                                   bbox.yMin};
            characters_.insert(std::pair<wchar_t, Character>(c, character));
           CheckGLError(glBindTexture(GL_TEXTURE_2D, 0)); 
            ++text_index_;
        }
 
       CheckGLError(glBindBuffer(GL_ARRAY_BUFFER, 0));
       CheckGLError(glBindVertexArray(0));
 
    }

    void RenderFont::RenderText(const std::wstring &text, GLfloat text_x, GLfloat text_y, GLfloat scale, const glm::vec4& color, bool should_load_text) 
    {        
        if (should_load_text)
        {
            LoadText(text);
        }

       CheckGLError(glUseProgram(shader_program_));
       CheckGLError(glUniform4f(glGetUniformLocation(shader_program_, "textColor"), color.r, color.g, color.b, color.a));
         
        TN_LOG(TN_LOG_ERROR) << "==================BEGIN RenderText======================="<< std::endl;
        TN_LOG(TN_LOG_ERROR) << "text.size:" << text.size() << "tex_x:" << text_x << ", text_y:" << text_y << " scale:" << scale << std::endl;
        GLfloat textHeight = 0;
        GLfloat x = text_x;
        GLfloat y = text_y;
        lines.clear();
        LineInfo line;
        line.begin_index = 0;
        // Calculate the overall height of the text so that it is vertically centered
        for (int index=0; index<text.size(); index++)
        {
            auto c = text[index];
            if (characters_.count(c) == 0)
            {
                continue;
            }

            auto& ch = characters_[c];
            int char_width = (ch.Advance >> 6) * scale;
            if (c == L'\n')
            {
                line.end_index = index - 1;
                lines.emplace_back(line);
                line.Reset();
                line.begin_index = index+1;
                line.end_index = line.begin_index;
                continue;
            }
            if ((line.width + char_width) > font_property_.max_line_width) {
                
                line.end_index = index-1;
                lines.emplace_back(line);
                line.Reset();
                line.begin_index = index;
                line.end_index = index;
            }

            line.width += char_width;
            line.ascend = glm::max(line.ascend, ch.Bearing.y * scale);
            line.descend = glm::max(line.descend, (ch.Size.y - ch.Bearing.y) * scale);
            line.height = line.ascend + line.descend;
            if(index == text.size()-1)
            {
                line.end_index = index;
                lines.emplace_back(line);
                line.Reset();
            }
        }
        for(auto &line : lines)
        {
            textHeight += line.height;
        }

        // Calculate the center position
        if (font_property_.horizontal_center) {
            x = text_x + (font_property_.show_width - font_property_.max_line_width) / 2.0f;
        }
        if (font_property_.vertical_center) {
            y = text_y + (font_property_.show_height - textHeight) / 2.0f;
            // if(font_property_.new_font_render)
            // {
            //     y -= font_property_.font_size / 2.0f; 
            // }
            // else
            // {
            //     y += font_property_.font_size / 2.0f; 
            // }
            // y = text_y + (font_property_.show_height - textHeight) / 2.0f;
        }

        TN_LOG(TN_LOG_ERROR) << "x:" << x << ", y:" << y << " textHeight:" << textHeight << std::endl;
        font_property_.Print();

        float begin_x = x;

        int cur_line_begin = 0;
        int cur_line_end = 0;
        bool start_cal_line_text = false;

        for(auto &line : lines)
        {
            if (font_property_.horizontal_center)
            {
                x = begin_x + (font_property_.max_line_width - line.width) * 0.5f;
            }
            else
            {
                x = begin_x;
            }
            // if (font_property_.vertical_center)
            // {
            //     if(font_property_.new_font_render)
            //     {
            //         y -= (font_property_.font_size  - 0) / 2.0;
            //     }
            //     else
            //     {
            //         y += (font_property_.font_size  - 0) / 2.0;
            //     }
            // }
            
            TN_LOG(TN_LOG_ERROR) << "x:" << x << ", y:" << y << " line.width:" << line.width << " font_property_.max_line_width:" << font_property_.max_line_width<< std::endl;
            for(int line_index=line.begin_index; line_index <= line.end_index; line_index++)
            {
                auto tc = text[line_index];
                if (characters_.count(tc) == 0)
                {
                    continue;
                }
                if (tc == L'\n')
                {
                    continue;
                }
                auto& ch = characters_[tc];
                
                GLfloat xpos = (x + ch.Bearing.x * scale) / viewport_w_;
                GLfloat ypos = 0;
                if(font_property_.new_font_render)
                {
                    // ypos = (y + font_property_.font_size * scale - (line.ascend - ch.Bearing.y) * scale) / viewport_h_;
                    ypos = (y + font_property_.font_size * scale - (ch.Size.y + ch.Ymin) * scale) / viewport_h_;
                    TN_LOG(TN_LOG_ERROR) << "x:" << x << ", y:" << y << " ch.Size.y:" << ch.Size.y << " ch.Ymin:" << ch.Ymin 
                    << " font_property_.font_size:" << font_property_.font_size
                    << "chary:" << (y + font_property_.font_size - (ch.Size.y + ch.Ymin) * scale)
                    << std::endl;
                }
                else
                {
                    ypos = (y - (ch.Size.y + ch.Ymin) * scale) / viewport_h_;
                }

                GLint loc =CheckGLError(glGetUniformLocation(shader_program_,"uImageBegin"));
                CheckGLError(glUniform2f(loc, xpos, ypos));

                GLfloat w = (ch.Size.x * scale) / viewport_w_;
                GLfloat h = (ch.Size.y * scale) / viewport_h_;

                loc =CheckGLError(glGetUniformLocation(shader_program_,"uImageSize"));
                CheckGLError(glUniform2f(loc, w, h)); 

                GLfloat vertices[6][4] = {
                    {0.0, 0.0, 0.0, 0.0},
                    {0.0, 1.0, 0.0, 1.0},
                    {1.0, 1.0, 1.0, 1.0},

                    {0.0, 0.0, 0.0, 0.0},
                    {1.0, 1.0, 1.0, 1.0},
                    {1.0, 0.0, 1.0, 0.0}};  
                CheckGLError(glActiveTexture(GL_TEXTURE0));
                CheckGLError(glBindVertexArray(vao_));
                CheckGLError(glBindTexture(GL_TEXTURE_2D, ch.TextureID)); 
                CheckGLError(glBindBuffer(GL_ARRAY_BUFFER, vbo_));
                CheckGLError(glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices)); 
                CheckGLError(glBindBuffer(GL_ARRAY_BUFFER, 0)); 
                CheckGLError(glDrawArrays(GL_TRIANGLES, 0, 6)); 
                CheckGLError(glBindVertexArray(0));
                int line_char_width = (ch.Advance >> 6) * scale;
                x += line_char_width; // Bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
            }

            x = begin_x;
            y += font_property_.font_size + font_property_.line_space;

        }
    
    }

    void RenderFont::Render()
    {

        GLint data[4] = {0};
        CheckGLError(glGetIntegerv(GL_VIEWPORT, data));
        viewport_w_ = data[2];
        viewport_h_ = data[3];
        if(viewport_w_ <= 0)
        {
            viewport_w_ = 1;
        }
        if(viewport_h_ <= 0)
        {
            viewport_h_ = 1;
        }

       CheckGLError(glEnable(GL_CULL_FACE));
       CheckGLError(glEnable(GL_BLEND));
       CheckGLError(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

        bool is_active = is_active_.load();
        if (is_active)
        { 
            std::wstring to_be_render_content; 
            bool content_changed = false;
            {
                std::lock_guard<std::mutex> lock(content_mtx_); 
                to_be_render_content = content_;
                content_changed = content_changed_;
                content_changed_ = false;
            }

            if (to_be_render_content.empty())
            {
                TN_LOG(TN_LOG_INFO) << "to_be_render_content.empty()" << std::endl;
                return ;
            }
            // shadow
            if(font_property_.shadow && false)
            {
                RenderText(to_be_render_content, x_+2, y_+2, 1, glm::vec4(0.0, 0.0, 0.0, 0.56), content_changed);
                RenderText(to_be_render_content, x_+4, y_+4, 1, glm::vec4(0.0, 0.0, 0.0, 0.26), content_changed);
            }
            // main font
            RenderText(to_be_render_content, x_, y_, 1, font_property_.text_color, content_changed);
        } 

        if(font_property_.debug)
        {
            RenderRectangle(x_, y_, font_property_.show_width, font_property_.show_height);


            for(int i=0; i<20; i++)
            {
                for(int j=0; j<20; j++)
                {
                    // RenderRectangle(i * 100, j*100, font_property_.show_width, font_property_.show_height);
                }
            }
        }

       CheckGLError(glDisable(GL_CULL_FACE));
    }

    void RenderFont::Show()
    {
        is_active_.store(true);
    }

    void RenderFont::Hide()
    {
        is_active_.store(false);
    }    

    void RenderFont::UpdateFontContent(std::wstring content)
    {
        {
            std::lock_guard<std::mutex> lock(content_mtx_);

            auto content_changed = !(content == content_);    

            if (content_changed)
            {
                content_ = content;
                content_changed_ = true;
            }
        } 
    }

    void RenderFont::SetLeft(float left)
    {
        x_ = left;
    }

    void RenderFont::SetTop(float top)
    {
        y_ = top;
    }


void RenderFont::RenderRectangle(float left, float top, float width, float height) {
    TN_LOG(TN_LOG_ERROR) << "==========================BEGIN render text rect=========================="<< std::endl;
    TN_LOG(TN_LOG_ERROR) << "left:" <<left << ", top:" << top << " width:" << width << " height:" << height << std::endl;

    left = 2 * left / viewport_w_ - 1;
    top = viewport_h_ - top;
    top = 2 * top / viewport_h_ - 1;
    width = 2 * width / viewport_w_;
    height = 2 * height / viewport_h_;

    float bottomLeftX = left;
    float bottomLeftY = top - height;
    float topRightX = left + width;
    float topRightY = top;
    float bottomRightX = topRightX;
    float bottomRightY = bottomLeftY;

    // 顶点数据：位置和颜色 (x, y, r, g, b)
    float vertices[] = {
        // 位置       // 颜色
        bottomLeftX, bottomLeftY, 1.0f, 0.0f, 0.0f,  // 左下角，红色
        bottomRightX, bottomRightY, 0.0f, 1.0f, 0.0f, // 右下角，绿色
        topRightX, topRightY, 0.0f, 0.0f, 1.0f,  // 右上角，蓝色
        left, top, 1.0f, 1.0f, 0.0f,   // 左上角，黄色
        (bottomLeftX + bottomRightX)/2.0, bottomRightY, 1.0, 1.0, 1.0,//下中心
        (bottomLeftX + bottomRightX)/2.0, top, 1.0, 1.0, 1.0,//上中心
        left, (top + bottomRightY)/2.0, 1.0, 1.0, 0.0,//左中心
        topRightX, (top + bottomRightY)/2.0, 1.0, 1.0, 0.0,//左中心
    };

    // 顶点索引，用于绘制两个三角形构成矩形
    // unsigned int indices[] = {
    //     0, 1, 2,  // 第一个三角形
    //     0, 2, 3   // 第二个三角形
    // };
      unsigned int indices[] = {
        0, 1, 
        1, 2,  
        2, 3,
        3, 0,
        4, 5,
        6, 7
    };

    // 创建 VAO 和 VBO
    GLuint VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    // 绑定 VAO
    glBindVertexArray(VAO);

    // 绑定并设置 VBO
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // 绑定并设置 EBO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // 位置属性
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // 颜色属性
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // 创建并使用着色器程序
    std::string shader_path = ConfigManager::Instance()->ShaderPath();
    unsigned int line_shader_program;
    if(tn::stitching::ShaderUtil::LoadShader(shader_path + "line.vert", shader_path + "line.frag", line_shader_program) != 0)
    {
        TN_LOG(TN_LOG_ERROR) << "shader_path:" << shader_path << ", " << shader_path + "line.vert" << ", " << shader_path + "line.frag"<< std::endl;
        TN_LOG(TN_LOG_ERROR) << "==========================RETURN render text rect=========================="<< std::endl;
        return;
    }
    glUseProgram(line_shader_program);


    // 绘制矩形
    glBindVertexArray(VAO);
    glDrawElements(GL_LINES, sizeof(indices), GL_UNSIGNED_INT, 0);

    // 解绑 VAO 和 VBO
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // 删除对象
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    TN_LOG(TN_LOG_ERROR) << "==========================END render text rect=========================="<< std::endl;
}
}
}
