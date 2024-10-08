#ifndef RENDER_FONT_H
#define RENDER_FONT_H

#include <mutex>
#include <atomic>
#include <iostream>
#include <map>
#include <string>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include <boost/filesystem.hpp>

#ifndef debug_font
#include "GLES2/gl2.h"
#else
#include <glad/glad.h>
#endif



namespace tn
{
namespace stitching
{
    struct Character
    {
        unsigned int TextureID; // ID handle of the glyph texture
        glm::ivec2 Size;        // Size of glyph
        glm::ivec2 Bearing;     // Offset from baseline to left/top of glyph
        unsigned int Advance;   // Horizontal offset to advance to next glyph
        signed long Ymin;
        void Print();
    };

    struct LineInfo
    {
        int begin_index{0};
        int end_index{0};
        float width{0};
        float height{0};
        float ascend{0};
        float descend{0};
        void Reset()
        {
            begin_index = 0;
            end_index = 0;
            width = 0;
            height = 0;
            ascend = 0;
            descend = 0;
        }
        void Print();
    };

    struct FontProperty
    {
        float show_width;
        float show_height;
        unsigned int font_size; 
        unsigned int max_font_num;
        unsigned int max_line_width;
        unsigned int line_space; 
        std::string font;
        bool shadow;
        unsigned int font_weight = 0;
        glm::vec4 text_color = {0.0, 0.0, 0.0, 0.56};
        bool horizontal_center{false};
        bool vertical_center{false};
        bool new_font_render{false};
        bool debug{false};

        // Constructor
        FontProperty(float _show_width, float _show_height, unsigned int _font_size, 
                    unsigned int _max_font_num, unsigned int _max_line_width, unsigned int _line_space, 
                    std::string _font, bool _shadow, unsigned int _font_weight = 0,
                    glm::vec4 _text_color = {0.0, 0.0, 0.0, 0.56}, bool _horizontal_center = false, 
                    bool _vertical_center = false, bool _new_font_render = false, bool _debug = false)
            : show_width(_show_width), show_height(_show_height), font_size(_font_size), max_font_num(_max_font_num),
            max_line_width(_max_line_width), line_space(_line_space), font(std::move(_font)), shadow(_shadow), 
            font_weight(_font_weight), text_color(_text_color), horizontal_center(_horizontal_center), 
            vertical_center(_vertical_center), new_font_render(_new_font_render), debug(_debug) {}
        void Print();
    };


    
    class RenderFont
    {
    public:  
        RenderFont(const std::wstring &content, const float& x, const float& y, const FontProperty &render_property);
        ~RenderFont()
        {
            if (font_loaded_)
            {
                FT_Done_Face(face_);
                FT_Done_FreeType(ft_);
            }
        }
        void Init();
        void LoadText(const std::wstring &text); 
        void RenderText(const std::wstring &text, float x, float y, float scale, const glm::vec4& color, bool should_load_text);  
        void Render();
        void Show();
        void Hide();
        void UpdateFontContent(std::wstring content);
        void SetLeft(float left);
        void SetTop(float top);
        float GetLeft() const {return x_;};
        float GetTop() const {return y_;}
        bool IsActive() const {return is_active_.load();}
        const FontProperty &GetFontProperty() { return font_property_; }
        void SetFontProperty(const FontProperty &font_peroperty);
        void RenderRectangle(float left, float top, float width, float height);
    private: 
        std::atomic<bool> init_done_;
        bool font_loaded_;
        bool content_changed_;
        GLuint shader_program_;
        std::map<wchar_t, Character> characters_;
        unsigned int vao_, vbo_;       
        std::wstring content_;
        float x_;
        float y_;

        int viewport_w_;
        int viewport_h_;

        std::atomic<bool> is_active_; 

        std::vector<unsigned int> texture_;
        std::mutex content_mtx_;

        int text_index_;
        FT_Library ft_;
        FT_Face face_;
        FontProperty font_property_;
        std::vector<LineInfo> lines;
    };
}
}
#endif