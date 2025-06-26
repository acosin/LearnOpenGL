#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <vector>

int width = 800, height = 600;
GLuint fbo, colorTex, depthTex, VAO, shaderProgram;
glm::mat4 projection, view, model;

const char* vertexShaderSource = R"(#version 320 es
layout (location = 0) in vec3 aPos;
uniform mat4 uMVP;
void main() {
    gl_Position = uMVP * vec4(aPos, 1.0);
})";

const char* fragmentShaderSource = R"(#version 320 es
precision mediump float;
out vec4 FragColor;
void main() {
    FragColor = vec4(1.0, 0.7, 0.3, 1.0);
})";

GLuint compileShader(GLenum type, const char* source) {
    GLuint s = glCreateShader(type);
    glShaderSource(s, 1, &source, nullptr);
    glCompileShader(s);
    return s;
}

void checkFramebuffer() {
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "FBO not complete!" << std::endl;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void setupFBO() {
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    // Color texture
    glGenTextures(1, &colorTex);
    glBindTexture(GL_TEXTURE_2D, colorTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTex, 0);

    // Depth texture
    glGenTextures(1, &depthTex);
    glBindTexture(GL_TEXTURE_2D, depthTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, width, height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, depthTex, 0);

    checkFramebuffer();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void setupCube() {
    float vertices[] = {
        -0.5,-0.5,-0.5, +0.5,-0.5,-0.5, +0.5,+0.5,-0.5,
        +0.5,+0.5,-0.5, -0.5,+0.5,-0.5, -0.5,-0.5,-0.5,
        -0.5,-0.5,+0.5, +0.5,-0.5,+0.5, +0.5,+0.5,+0.5,
        +0.5,+0.5,+0.5, -0.5,+0.5,+0.5, -0.5,-0.5,+0.5,
        -0.5,+0.5,+0.5, -0.5,+0.5,-0.5, -0.5,-0.5,-0.5,
        -0.5,-0.5,-0.5, -0.5,-0.5,+0.5, -0.5,+0.5,+0.5,
        +0.5,+0.5,+0.5, +0.5,+0.5,-0.5, +0.5,-0.5,-0.5,
        +0.5,-0.5,-0.5, +0.5,-0.5,+0.5, +0.5,+0.5,+0.5,
        -0.5,-0.5,-0.5, +0.5,-0.5,-0.5, +0.5,-0.5,+0.5,
        +0.5,-0.5,+0.5, -0.5,-0.5,+0.5, -0.5,-0.5,-0.5,
        -0.5,+0.5,-0.5, +0.5,+0.5,-0.5, +0.5,+0.5,+0.5,
        +0.5,+0.5,+0.5, -0.5,+0.5,+0.5, -0.5,+0.5,-0.5,
    };

    GLuint VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), nullptr);
    glEnableVertexAttribArray(0);
}

void renderToFBO() {
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glViewport(0, 0, width, height);
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.1, 0.1, 0.1, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    projection = glm::perspective(glm::radians(45.0f), (float)width/height, 0.1f, 100.0f);
    view = glm::lookAt(glm::vec3(1.5,1.5,1.5), glm::vec3(0,0,0), glm::vec3(0,1,0));
    model = glm::mat4(1.0f);
    glm::mat4 mvp = projection * view * model;

    glUseProgram(shaderProgram);
    GLint mvpLoc = glGetUniformLocation(shaderProgram, "uMVP");
    glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, glm::value_ptr(mvp));
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void mouse_button_callback(GLFWwindow*, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        double xpos, ypos;
        glfwGetCursorPos(glfwGetCurrentContext(), &xpos, &ypos);
        int flipY = height - (int)ypos;

        unsigned int depth = -1;
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glReadPixels((int)xpos, flipY, 1, 1, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT_24_8, &depth);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        GLenum err = glGetError();
        std::cout << "Depth at (" << xpos << "," << ypos << ") = " << depth << std::endl;
        if (err != GL_NO_ERROR) std::cerr << "glReadPixels error: 0x" << std::hex << err << std::endl;

        // 提取高 24 位：右移 8 位
unsigned int depth24 = depth >> 8;

// 转换为 [0.0, 1.0] 区间浮点数
float depthNormalized = depth24 / float(0xFFFFFF); // = 2^24 - 1 = 16777215
std::cout << "Raw depth = 0x" << std::hex << depth<< std::dec << ", normalized depth = " << depthNormalized << std::endl;

        if (depth >= 0.0f && depth <= 1.0f) {
            glm::vec3 screen(xpos, flipY, depth);
            glm::vec3 world = glm::unProject(screen, view * model, projection, glm::vec4(0, 0, width, height));
            std::cout << "World: " << world.x << ", " << world.y << ", " << world.z << std::endl;
        }
    }
}

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_DEPTH_BITS, 24);
    GLFWwindow* window = glfwCreateWindow(width, height, "FBO Depth", nullptr, nullptr);
    glfwMakeContextCurrent(window);
    gladLoadGLES2Loader((GLADloadproc)glfwGetProcAddress);
    glfwSetMouseButtonCallback(window, mouse_button_callback);

    GLuint vs = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
    GLuint fs = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vs); glAttachShader(shaderProgram, fs);
    glLinkProgram(shaderProgram);
    glDeleteShader(vs); glDeleteShader(fs);

    setupFBO();
    setupCube();

    while (!glfwWindowShouldClose(window)) {
        renderToFBO();

        // 显示渲染结果（直接把colorTex绘制到默认帧缓冲）
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClear(GL_COLOR_BUFFER_BIT);
        glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        glBlitFramebuffer(0, 0, width, height,
                          0, 0, width, height,
                          GL_COLOR_BUFFER_BIT, GL_NEAREST);
        glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

//######################################
//    对于GL_UNSIGNED_INT_24_8格式的深度模板缓冲区，获取后，如何将无符号整型深度转换为0~1之间的值   
// unsigned int depth = -1;
//         glBindFramebuffer(GL_FRAMEBUFFER, fbo);
//         glReadPixels((int)xpos, flipY, 1, 1, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT_24_8, &depth);
//         glBindFramebuffer(GL_FRAMEBUFFER, 0);


// ChatGPT said:
// 你使用的是 GL_UNSIGNED_INT_24_8 格式的深度+模板缓冲区（比如 GL_DEPTH24_STENCIL8 作为渲染缓冲区的格式），通过 glReadPixels 获取到了一个 32 位无符号整型值（depth），其中：

// 高 24 位 存储 深度值

// 低 8 位 存储 模板值

// 你现在需要将读取到的 unsigned int 的高 24 位深度值，转换为标准的 [0.0, 1.0] 深度值。

//  转换方法（提取高 24 位并归一化）
// cpp
// Copy
// Edit
// unsigned int depthRaw = ...; // 读取回来的值，格式为 GL_UNSIGNED_INT_24_8

// // 提取高 24 位：右移 8 位
// unsigned int depth24 = depthRaw >> 8;

// // 转换为 [0.0, 1.0] 区间浮点数
// float depthNormalized = depth24 / float(0xFFFFFF); // = 2^24 - 1 = 16777215

// 拓展说明
// 格式	位宽	描述
// GL_DEPTH24_STENCIL8	32 位	高 24 位是深度（Z），低 8 位是模板（Stencil）
// GL_UNSIGNED_INT_24_8	对应读取格式	用于读取 24+8 的复合缓冲

// 注意事项：
// 你不能用 GL_FLOAT 读取这种格式（否则 GL_INVALID_OPERATION）

// 不要用 GL_DEPTH_STENCIL 作为 glReadPixels() 的格式（ES 不支持）

// 必须手动解析出高位的深度值，不能直接作为 float 使用

// 如果使用 MSAA 帧缓冲，则不能直接读取，需先 blit 到中间 FBO
