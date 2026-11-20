#pragma once
#include "gl.hpp"
#include "math.hpp"
#include "maze.hpp"

struct Mesh { GLuint vao=0, buffer=0; GLsizei count=0; };
class Renderer {
public:
    Renderer();
    ~Renderer();
    void draw(const Maze&, Point marble, const Mat4& board, int width, int height);
private:
    Mesh cube, sphere;
    GLuint program;
    GLint model, color, viewProjection;
};
