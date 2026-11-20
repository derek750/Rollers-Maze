#include "renderer.hpp"
#include "physics.hpp"
#include "gl.hpp"
#include <GLFW/glfw3.h>
#include <algorithm>
#include <charconv>
#include <random>
#include <string>
#include <string_view>
#include <cstdio>
#include <stdexcept>

static void run(GLFWwindow* window,std::uint32_t seed) {
    glfwSetInputMode(window,GLFW_STICKY_KEYS,GLFW_TRUE);
    Renderer renderer;
    Maze maze=generateMaze(seed);
    std::mt19937 seeds(std::random_device{}());
    double elapsed=0, nextTitle=0;
    bool started=false;
    Mat4 spin=identity();
    bool newHeld=false;
    Marble ball{maze.start,{}};
    ExitState exit;
    bool resetHeld=false;
    Clock clock;
    Tilt tilt{}, anchor{};
    bool dragging=false;
    double originX=0, originY=0, last=glfwGetTime();
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        double now=glfwGetTime(), x,y;
        glfwGetCursorPos(window,&x,&y);
        bool reset=glfwGetKey(window,GLFW_KEY_R)==GLFW_PRESS;
        bool next=glfwGetKey(window,GLFW_KEY_N)==GLFW_PRESS;
        bool newRound=next && !newHeld;
        newHeld=next;
        if (newRound) {
            Maze candidate;
            do { seed=seeds(); candidate=generateMaze(seed); } while (candidate.cells==maze.cells);
            maze=std::move(candidate);
        }
        if (newRound || (reset && !resetHeld)) {
            ball={maze.start,{}};
            exit={}; clock={}; tilt={}; dragging=false; last=now;
            started=false; elapsed=0; nextTitle=0; spin=identity();
        }
        resetHeld=reset;
        bool held=glfwGetMouseButton(window,GLFW_MOUSE_BUTTON_LEFT)==GLFW_PRESS;
        if (held) started=true;
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
            if (exit.won) return;
            if (started) elapsed+=dt;
            Point before=ball.position;
            stepMaze(maze,ball,downhill(boardRotation(tilt)),dt,exit);
            Point movement=ball.position-before;
            float distance=std::sqrt(dot(movement,movement));
            if (distance>1e-7f) {
                Vec3 axis{movement.z/distance,0,-movement.x/distance};
                spin=rotation(distance/marbleRadius,axis)*spin;
            }
        });
        if (now>=nextTitle) {
            char title[180];
            std::snprintf(title,sizeof(title),"Rollers Maze  |  %s%.1f s  |  Seed %u  |  Drag to tilt   R restart   N new",
                          exit.won ? "Escaped!  " : "",elapsed,seed);
            glfwSetWindowTitle(window,title);
            nextTitle=now+.1;
        }
        last=now;
        int w,h;
        glfwGetFramebufferSize(window,&w,&h);
        renderer.draw(maze,ball.position,boardRotation(tilt),spin,w,h);
        glfwSwapBuffers(window);
    }
}

int main(int argc,char** argv) {
    std::uint32_t seed=0;
    if (argc==1) seed=std::random_device{}();
    else {
        std::string_view value=argv[argc-1];
        auto result=std::from_chars(value.data(),value.data()+value.size(),seed);
        if (argc!=3 || std::string_view(argv[1])!="--seed" || result.ec!=std::errc{} ||
            result.ptr!=value.data()+value.size()) {
            std::fprintf(stderr,"Usage: rollers [--seed 0..4294967295]\n");
            return 2;
        }
    }
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
            run(window,seed);
            status = 0;
        } catch (const std::exception& error) {
            std::fprintf(stderr, "%s\n", error.what());
        }
        glfwDestroyWindow(window);
    }
    glfwTerminate();
    return status;
}
