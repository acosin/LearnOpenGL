#include <iostream>
#include <map>
#include <string>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <learnopengl/filesystem.h>
#include <learnopengl/shader.h>


#include "config_manager.h"

#pragma message(def debug_font)





#include "cmdline.h"


// #define CheckGLError(glFunc) \
//     glFunc;\
//     tn::stitching::InnerCheckGLError(__FILE__, __LINE__);
// #else
// #define CheckGLError(glFunc) glFunc;
// #endif

#include "shader_util.h"
#include "camera_renderer.h"
#include "texture_util.h"

using namespace tn::stitching;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

auto camera_renderer_ = std::make_shared<CameraRenderer>();


struct InputTextureKey
{
    Position pos = Position::kInvalidPosition;
    int dma_buf_fd = -1;

    bool IsValid() const
    {
        return ((pos != Position::kInvalidPosition) &&
                (dma_buf_fd != -1));
    }

    bool operator < (const InputTextureKey& rhs) const
    {
        if (pos != rhs.pos)
        {
            return (pos < rhs.pos);
        }

        return dma_buf_fd < rhs.dma_buf_fd;
    }

    bool operator > (const InputTextureKey& rhs) const
    {
        if (pos != rhs.pos)
        {
            return (pos > rhs.pos);
        }

        return dma_buf_fd > rhs.dma_buf_fd;
    }
};
struct TextureObj
{
    TextureObj(GLuint t, GLuint t1, bool a = true)
        : texture(t)
        , texture_1(t1)
        , active(a) 
        {}
    TextureObj() {}
    operator GLuint() { return texture; }
    GLuint texture = 0;
    GLuint texture_1 = 0;
    bool active = true;
    bool active_1 = true;
};
std::map<InputTextureKey, TextureObj> input_textures_;
bool InitRendering()
{
    do
    {// create input-textures from non-egl


        bool is_nv12_input = (ImageFormat::kNv12 == ConfigManager::Instance()->InputImageFormat());
        InputTextureKey key;

        key.pos = Position::kFront;
        key.dma_buf_fd = 0;
        input_textures_[key].texture = TextureUtil::Instance()->GenerateTexture(GL_TEXTURE_2D);
        if (is_nv12_input)
        {
            input_textures_[key].texture_1 = TextureUtil::Instance()->GenerateTexture(GL_TEXTURE_2D);
        }

        key.pos = Position::kRear;
        key.dma_buf_fd = 1;
        input_textures_[key].texture = TextureUtil::Instance()->GenerateTexture(GL_TEXTURE_2D);
        if (is_nv12_input)
        {
            input_textures_[key].texture_1 = TextureUtil::Instance()->GenerateTexture(GL_TEXTURE_2D);
        }

        key.pos = Position::kLeft;
        key.dma_buf_fd = 2;
        input_textures_[key].texture = TextureUtil::Instance()->GenerateTexture(GL_TEXTURE_2D);
        if (is_nv12_input)
        {
            input_textures_[key].texture_1 = TextureUtil::Instance()->GenerateTexture(GL_TEXTURE_2D);
        }

        key.pos = Position::kRight;
        key.dma_buf_fd = 3;
        input_textures_[key].texture = TextureUtil::Instance()->GenerateTexture(GL_TEXTURE_2D);
        if (is_nv12_input)
        {
            input_textures_[key].texture_1 = TextureUtil::Instance()->GenerateTexture(GL_TEXTURE_2D);
        }
    } while(false);

    return true;
}

std::map<Position, std::pair<GLuint, GLuint>> GetRenderTextures()
{
    std::map<Position, std::pair<GLuint, GLuint>> ret;
    for (const auto& pos : input_textures_)
    {
        if (!pos.first.IsValid())
        {
            continue ;
        }

        auto& pos_sec = pos.second;
        if (!pos_sec.active)
        {
            continue ;
        }

        ret[pos.first.pos] = std::make_pair(0, 0);
        ret[pos.first.pos].first = pos.second.texture;
        if (pos.second.active_1)
        {
            ret[pos.first.pos].second = pos.second.texture_1;
        }
    }
    return ret;
}

int main(int argc, char* argv[], char* envp[])
{
#if 1
    // create a parser
    cmdline::parser a;

    // add specified type of variable.
    // 1st argument is long name
    // 2nd argument is short name (no short name if '\0' specified)
    // 3rd argument is description
    // 4th argument is mandatory (optional. default is false)
    // 5th argument is default value  (optional. it used when mandatory is false)
    // a.add<int>("left", 'l', "left", false, 0);
    // a.add<int>("top", '\0', "top", false, 0);
    //  a.add<int>("show-width", '\0', "show-width", false, 100);
    // a.add<int>("show-height", '\0', "show-height", false, 100);
    // a.add<int>("line-width", '\0', "line-width", false, 100);
    // a.add<int>("line-space", '\0', "line-space", false, 5);
    // a.add<bool>("horizontal-center", '\0', "horizontal-center", false, false);
    // a.add<bool>("vertical-center", '\0', "vertical-center", false, false);
    // a.add<bool>("shadow", '\0', "shadow", false, false);
    // a.add<int>("font-size", '\0', "font-size", false, 48);
    // a.add<int>("max-font-num", '\0', "max-font-num", false, 0);
    a.add<bool>("render_thumb_nail", '\0', "render_thumb_nail", false, false);
    a.add<int>("org_width", '\0', "org_width", false, 1000);
    a.add<int>("org_height", '\0', "org_height", false, 1000);
    a.add<int>("width", '\0', "width", false, 500);
    a.add<int>("height", '\0', "height", false, 500);
    a.add<bool>("is_apa", '\0', "is apa", false, false);


    // a.add<std::wstring>("content", '\0', "content", false, L"");
    
    // a.add<int>("from", '\0', "from frame index", false, 0);

    // a.add<int>("to", '\0', "to frame index", false, 100);

    // a.add<std::string>("image-path", 'i', "stitching image path", false, "");

    // a.add<int>("frame-rate", 'r', "frame rate", false, 20);
 
    // a.add<bool>("read-from-folder", '\0', "read from folder", false, false);

    // a.add<bool>("small-screen", 's', "change screen mode:small or big", false, false);

    // a.add<float>("factor", '\0', "window size factor", false, 1.0);

    // a.add<std::string>("config-path", 'c', "config  path", true, "");
    // a.add<std::string>("config-file", '\0', "config  file(.json)", false, "");
    // a.add<int>("msaa", '\0', "multisample AA", false, 1);

    // a.add<std::string>("platform", 'p', "select platform:android, linux-ubuntu-1804, linux-ubuntu-faw-c801, linux-ubuntu-saic, windows-faw-801, windows-saic", true, "linux-ubuntu-faw-c801");
    // a.add<std::string>("mode", 'm', "select mode:avm, apa", false, "avm");

    // //simuate data:
    // a.add<int>("from", '\0', "replay avm begin index", false, 0);
    // a.add<int>("to", '\0', "replay avm end index", false, 100);
    // // a.add<bool>("loop", '\0', "is loop replay avm images", false, false);
    // a.add<int>("frame-rate", '\0', "replay frame rate", false, 20);

    

    a.parse_check(argc, argv);

    // std::string plat = a.get<std::string>("platform");
    bool render_thumb_nail = a.get<bool>("render_thumb_nail");
    int org_width = a.get<int>("org_width");
    int org_height = a.get<int>("org_height");
    int width = a.get<int>("width");
    int height = a.get<int>("height");
    bool is_apa = a.get<bool>("is_apa");

#endif
    
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    
    InitRendering();
    // OpenGL state
    // ------------
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // compile and setup the shader
    // ----------------------------
    Shader shader("text.vs", "text.fs");
    glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(SCR_WIDTH), 0.0f, static_cast<float>(SCR_HEIGHT));
    shader.use();
    glUniformMatrix4fv(glGetUniformLocation(shader.ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));


    // render loop
    while (!glfwWindowShouldClose(window))
    {
        // input
        // -----
        processInput(window);

        // render
        // ------
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        bool eglimage_input_ = false;
        if(render_thumb_nail)
        {

            camera_renderer_->RenderThumbnail(eglimage_input_ ? GL_TEXTURE_EXTERNAL_OES : GL_TEXTURE_2D,
                                          Position::kFront,
                                          org_width,
                                          org_height,
                                          width, height,
                                          GetRenderTextures());
            
            camera_renderer_->RenderThumbnail(eglimage_input_ ? GL_TEXTURE_EXTERNAL_OES : GL_TEXTURE_2D,
                                          Position::kRear,
                                          org_width,
                                          org_height,
                                          width, height,
                                          GetRenderTextures());

            
            camera_renderer_->RenderThumbnail(eglimage_input_ ? GL_TEXTURE_EXTERNAL_OES : GL_TEXTURE_2D,
                                          Position::kLeft,
                                          org_width,
                                          org_height,
                                          width, height,
                                          GetRenderTextures());

            camera_renderer_->RenderThumbnail(eglimage_input_ ? GL_TEXTURE_EXTERNAL_OES : GL_TEXTURE_2D,
                                          Position::kRight,
                                          org_width,
                                          org_height,
                                          width, height,
                                          GetRenderTextures());

        }
        else
        {
            camera_renderer_->Render(RenderType::kMediumAvm, eglimage_input_ ? GL_TEXTURE_EXTERNAL_OES : GL_TEXTURE_2D, GetRenderTextures());
            camera_renderer_->Render(RenderType::kFarAvm, eglimage_input_ ? GL_TEXTURE_EXTERNAL_OES : GL_TEXTURE_2D, GetRenderTextures());
            if(is_apa)
            {
                camera_renderer_->Render(RenderType::kNearAvm, eglimage_input_ ? GL_TEXTURE_EXTERNAL_OES : GL_TEXTURE_2D, GetRenderTextures());
            }
            else
            {
                camera_renderer_->Render(RenderType::kNearAvmVPA, eglimage_input_ ? GL_TEXTURE_EXTERNAL_OES : GL_TEXTURE_2D, GetRenderTextures());
            }
            camera_renderer_->Render(RenderType::kCustomAvm1, eglimage_input_ ? GL_TEXTURE_EXTERNAL_OES : GL_TEXTURE_2D, GetRenderTextures());
        }
        // TN_LOG(X) << "buffers and poll IO events (keys pressed/released, mouse moved etc.)" << std::endl;

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// #include "render_font.cpp"
// #include "shader_util.cpp"