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
#include "render_font.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);
void RenderText(Shader &shader, std::string text, float x, float y, float scale, glm::vec3 color);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;



using namespace tn::stitching;

ConfigManager* ConfigManager::instance_ = nullptr;

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
    a.add<int>("left", 'l', "left", false, 0);
    a.add<int>("top", '\0', "top", false, 0);
     a.add<int>("show-width", '\0', "show-width", false, 100);
    a.add<int>("show-height", '\0', "show-height", false, 100);
    a.add<int>("line-width", '\0', "line-width", false, 100);
    a.add<int>("line-space", '\0', "line-space", false, 5);
    a.add<bool>("horizontal-center", '\0', "horizontal-center", false, false);
    a.add<bool>("vertical-center", '\0', "vertical-center", false, false);
    a.add<bool>("shadow", '\0', "shadow", false, false);
    a.add<int>("font-size", '\0', "font-size", false, 48);
    a.add<int>("max-font-num", '\0', "max-font-num", false, 0);
    a.add<bool>("new-font-render", '\0', "new font render", false, false);
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



	// find path to font
    std::string font_name = FileSystem::getPath("resources/fonts/QimiaoType-Regular.ttf");
    if (font_name.empty())
    {
        std::cout << "ERROR::FREETYPE: Failed to load font_name" << std::endl;
        return -1;
    }
	


    const auto image = "";
    const auto max_font_num = a.get<int>("max-font-num");
    const auto show_width = a.get<int>("show-width");                     
    const auto show_height = a.get<int>("show-height");
    const auto font_size = a.get<int>("font-size");//48
    const auto height = 64;
    const auto horizontal_center = a.get<bool>("horizontal-center");                   
    const auto vertical_center = a.get<bool>("vertical-center");  
    
    auto line_width = a.get<int>("line-width");                   
    auto line_space = a.get<int>("line-space");
    const auto font_weight = 64;
    const auto font_type = "";
    const auto shadow = a.get<bool>("shadow");  
    const auto new_font_render = a.get<bool>("new-font-render");
    const auto debug = true;
    auto color = glm::vec4(1, 1, 1, 1);
    // std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
    // std::wstring wide_str = conv.from_bytes(image);
    FontProperty font_property(show_width, show_height, 
            font_size, max_font_num, 
            line_width, line_space, 
            font_type, shadow, font_weight, 
            color,
            horizontal_center,
            vertical_center,
            new_font_render,
            debug
            );
    std::wstring wide_str = L"当你在图形计算领域冒险到了一定阶段以后你可能会想使用OpenGL来绘制文本。";
    // wide_str = L"GPT-4 is OpenAI’s most advanced system, producing safer and more useful responses";
    // wide_str = L"GPT-4 is OpenAI’s";
    wide_str = L"距离过近258cmghjkHM";
    // wide_str = L"距离过近";
    wide_str = L"258cm";
    float left = a.get<int>("left");
    float top = a.get<int>("top");
    RenderFont render_font(wide_str, left, top, font_property);
    render_font.Init();
    render_font.Show();
    // render loop
    // -----------
    // std::string text = "当你在图形计算领域冒险到了一定阶段以后你可能会想使用OpenGL来绘制文本。然而，可能与你想象的并不一样，使用像OpenGL这样的底层库来把文本渲染到屏幕上并不是一件简单的事情。";
    //text = "GPT-4 is OpenAI’s most advanced system, producing safer and more useful responses";
    while (!glfwWindowShouldClose(window))
    {
        // input
        // -----
        processInput(window);

        // render
        // ------
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        // TN_LOG(X) << "buffers and poll IO events (keys pressed/released, mouse moved etc.)" << std::endl;
        render_font.Render();
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