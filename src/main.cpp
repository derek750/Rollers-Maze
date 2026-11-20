#include "gl.hpp"
#include <GLFW/glfw3.h>
#include <algorithm>
#include <cstdio>
#include <stdexcept>

static void run(GLFWwindow* window) {
    GLuint program = makeProgram(R"(#version 330 core
uniform vec2 scale;
void main() {
    vec2 p[3] = vec2[3](vec2(-.6,-.45),vec2(.6,-.45),vec2(0,.6));
    gl_Position=vec4(p[gl_VertexID]*scale,0,1);
})", R"(#version 330 core
out vec4 color;
void main(){color=vec4(.95,.6,.18,1);})");
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glUseProgram(program);
    while (!glfwWindowShouldClose(window)) {
        int w, h;
        glfwGetFramebufferSize(window, &w, &h);
        glViewport(0, 0, w, h);
        if (w && h) {
            float side = std::min(w, h);
            glUniform2f(glGetUniformLocation(program,"scale"), side/w, side/h);
            glClearColor(.06f,.10f,.14f,1);
            glClear(GL_COLOR_BUFFER_BIT);
            glDrawArrays(GL_TRIANGLES, 0, 3);
        }
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glDeleteVertexArrays(1, &vao);
    glDeleteProgram(program);
}

int main() {
    glfwSetErrorCallback([](int, const char* message){ std::fprintf(stderr, "GLFW: %s\n", message); });
    if (!glfwInit()) return 1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
    GLFWwindow* window = glfwCreateWindow(1000, 760, "Rollers Maze", nullptr, nullptr);
    int status = 1;
    if (window) {
        glfwMakeContextCurrent(window);
        glfwSwapInterval(1);
        glewExperimental = GL_TRUE;
        try {
            GLenum error = glewInit();
            if (error != GLEW_OK) throw std::runtime_error(reinterpret_cast<const char*>(glewGetErrorString(error)));
            run(window);
            status = 0;
        } catch (const std::exception& error) {
            std::fprintf(stderr, "%s\n", error.what());
        }
        glfwDestroyWindow(window);
    }
    glfwTerminate();
    return status;
}
