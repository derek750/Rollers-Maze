#include "renderer.hpp"
#include "physics.hpp"
#include "gl.hpp"
#include <GLFW/glfw3.h>
#include <algorithm>
#include <cstdio>
#include <stdexcept>

static void run(GLFWwindow* window) {
    Renderer renderer;
    Maze maze=fixedMaze();
    Marble ball{maze.start,{}};
    Clock clock;
    Tilt tilt{}, anchor{};
    bool dragging=false;
    double originX=0, originY=0, last=glfwGetTime();
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        double now=glfwGetTime(), x,y;
        glfwGetCursorPos(window,&x,&y);
        bool held=glfwGetMouseButton(window,GLFW_MOUSE_BUTTON_LEFT)==GLFW_PRESS;
        if (held && !dragging) {
            originX=x; originY=y; anchor=tilt;
        }
        dragging=held;
        if (held) {
            tilt.pitch=std::clamp(anchor.pitch+float(y-originY)*.0025f,-maxTilt,maxTilt);
            tilt.roll=std::clamp(anchor.roll-float(x-originX)*.0025f,-maxTilt,maxTilt);
        }
        clock.update(now-last,[&](float dt) {
            if (!held) {
                tilt.pitch*=std::exp(-8*dt);
                tilt.roll*=std::exp(-8*dt);
            }
            advance(ball,downhill(boardRotation(tilt)),dt);
        });
        last=now;
        int w,h;
        glfwGetFramebufferSize(window,&w,&h);
        renderer.draw(maze,ball.position,boardRotation(tilt),w,h);
        glfwSwapBuffers(window);
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
