#ifndef FONT_RENDER_H
#define FONT_RENDER_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <ft2build.h>
#include FT_FREETYPE_H
#include <map>
#include <string>
#include <iostream>
#include <vector>
#include <mutex>

struct Character {
    GLuint TextureID;   // 字符纹理ID
    glm::ivec2 Size;    // 字符大小
    glm::ivec2 Bearing; // 字符基线到图像左部和顶部的距离
    GLuint Advance;     // 从当前字符到下一个字符的水平偏移
};

class TextRender {
public:
    TextRender(const std::string& fontPath, GLuint fontSize, GLuint shaderProgram)
        : fontSize(fontSize), shaderProgram(shaderProgram) {
        // 初始化 FreeType 库
        if (FT_Init_FreeType(&ft)) {
            std::cerr << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
            return;
        }

        // 加载字体
        if (FT_New_Face(ft, fontPath.c_str(), 0, &face)) {
            std::cerr << "ERROR::FREETYPE: Failed to load font" << std::endl;
            return;
        }
        FT_Set_Pixel_Sizes(face, 0, fontSize);

        // 禁用字节对齐限制
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        // 配置 VAO/VBO
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*)0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        // 启用混合
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    ~TextRender() {
        FT_Done_Face(face);
        FT_Done_FreeType(ft);
        // 清理所有字符的纹理
        for (auto& pair : Characters) {
            glDeleteTextures(1, &pair.second.TextureID);
        }
    }

    void RenderText(const std::wstring& text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color, GLfloat screenWidth, GLfloat screenHeight, bool horizontalCenter = false, bool verticalCenter = false, GLfloat maxLineWidth = 0.0f) {
        glUseProgram(shaderProgram);

        // 设置着色器的文字颜色
        glUniform3f(glGetUniformLocation(shaderProgram, "textColor"), color.x, color.y, color.z);
        glActiveTexture(GL_TEXTURE0);
        glBindVertexArray(VAO);

        // 初始化顶点数据
        std::vector<GLfloat> vertices;
        GLfloat currentLineWidth = 0;
        GLfloat textHeight = 0;
        GLfloat lineHeight = 0;

        // 计算文本整体高度以便垂直居中
        for (auto c : text) {
            if (c == L'\n' || (currentLineWidth + (GetCharacter(c).Advance >> 6) * scale) > maxLineWidth) {
                textHeight += lineHeight;
                currentLineWidth = 0;
                lineHeight = 0;
                if (c == L'\n') continue;
            }
            Character ch = GetCharacter(c);
            currentLineWidth += (ch.Advance >> 6) * scale;
            lineHeight = std::max(lineHeight, ch.Size.y * scale);
        }
        textHeight += lineHeight;

        // 计算居中位置
        if (horizontalCenter) {
            x = (screenWidth - maxLineWidth) / 2.0f;
        }
        if (verticalCenter) {
            y = (screenHeight + textHeight) / 2.0f;
        }

        currentLineWidth = 0;
        lineHeight = 0;

        // 生成所有字符的顶点数据
        for (auto c : text) {
            if (c == L'\n' || (currentLineWidth + (GetCharacter(c).Advance >> 6) * scale) > maxLineWidth) {
                y -= lineHeight;
                currentLineWidth = 0;
                lineHeight = 0;
                if (c == L'\n') continue;
            }
                Character ch = GetCharacter(c);
                GLfloat xpos = x + currentLineWidth + ch.Bearing.x * scale;
                GLfloat ypos = y - (ch.Size.y - ch.Bearing.y) * scale;
                GLfloat w = ch.Size.x * scale;
                GLfloat h = ch.Size.y * scale;

                // 将当前字符的顶点数据添加到 vertices 中
                vertices.insert(vertices.end(), {
                    xpos,     ypos + h,   0.0f, 0.0f,
                    xpos,     ypos,       0.0f, 1.0f,
                    xpos + w, ypos,       1.0f, 1.0f,

                    xpos,     ypos + h,   0.0f, 0.0f,
                    xpos + w, ypos,       1.0f, 1.0f,
                    xpos + w, ypos + h,   1.0f, 0.0f
                });

                currentLineWidth += (ch.Advance >> 6) * scale;
                lineHeight = std::max(lineHeight, ch.Size.y * scale);

                std::cout << "\nc:" << c << ", w:" << w << ", h:" << h << std::endl;
                for(int index=vertices.size() - 24; index<vertices.size(); index++)
                {
                    std::cout << "vertices[" << index << "] = " <<  vertices[index] << std::endl;
                }
        }

        

        // 批量传输顶点数据到 GPU
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), vertices.data(), GL_DYNAMIC_DRAW);
        // glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(GLfloat), vertices.data());
        // glBindBuffer(GL_ARRAY_BUFFER, 0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*)0);

        // 渲染所有字符
        GLuint textureID = 0;
        int visible_text_index = 0;
        for (size_t i = 0; i < text.size(); ++i) {
            if (text[i] == L'\n') continue;
            

            Character ch = GetCharacter(text[i]);
            std::cout << "glDrawArrays:" << text[i] << ":" << ch.Size.x << ", " << ch.Size.y << std::endl;
            if (textureID != ch.TextureID) {
                textureID = ch.TextureID;
                glBindTexture(GL_TEXTURE_2D, textureID);
            }
            glDrawArrays(GL_TRIANGLES, visible_text_index * 6, 6);
            visible_text_index++;
        }

        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

private:
    Character GetCharacter(wchar_t c) {
        // 查找字符是否已加载
        auto it = Characters.find(c);
        if (it != Characters.end()) {
            return it->second;
        }

        // 如果未加载，则懒加载该字符
        if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
            std::cerr << "ERROR::FREETYPE: Failed to load Glyph" << std::endl;
            return Character{};
        }

        // 生成纹理
        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RED,
            face->glyph->bitmap.width,
            face->glyph->bitmap.rows,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE,
            face->glyph->bitmap.buffer
        );

        // 设置纹理选项
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // 存储字符信息
        Character character = {
            texture,
            glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            face->glyph->advance.x
        };

        // 加入字符到 Characters 中
        Characters.insert(std::pair<wchar_t, Character>(c, character));

        std::cout << c << ":" << character.Size.x << ", " << character.Size.y << std::endl; 
        return character;
    }

    GLuint VAO, VBO;
    FT_Library ft;
    FT_Face face;
    GLuint shaderProgram;
    GLuint fontSize;
    std::map<wchar_t, Character> Characters;
};


#endif