#include "renderer.hpp"
#include "gl.hpp"
#include <GLFW/glfw3.h>
#include <algorithm>
#include <cstdio>
#include <stdexcept>

static void run(GLFWwindow* window) {
    Renderer renderer;
    Maze maze=fixedMaze();
    while (!glfwWindowShouldClose(window)) {
        int w,h;
        glfwGetFramebufferSize(window,&w,&h);
        renderer.draw(maze,maze.start,identity(),w,h);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
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
