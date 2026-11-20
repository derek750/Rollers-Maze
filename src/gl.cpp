#include "gl.hpp"
#include <stdexcept>
#include <string>

namespace {
void check(GLuint object, bool program, const char* stage) {
    GLint ok, size;
    if (program) glGetProgramiv(object, GL_LINK_STATUS, &ok);
    else glGetShaderiv(object, GL_COMPILE_STATUS, &ok);
    if (ok) return;
    if (program) glGetProgramiv(object, GL_INFO_LOG_LENGTH, &size);
    else glGetShaderiv(object, GL_INFO_LOG_LENGTH, &size);
    std::string log(size + 1, '\0');
    if (program) glGetProgramInfoLog(object, size, nullptr, log.data());
    else glGetShaderInfoLog(object, size, nullptr, log.data());
    throw std::runtime_error(std::string(stage) + ": " + log.c_str());
}
GLuint compile(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);
    check(shader, false, type == GL_VERTEX_SHADER ? "Vertex shader" : "Fragment shader");
    return shader;
}
}
GLuint makeProgram(const char* vertex, const char* fragment) {
    GLuint program = glCreateProgram();
    for (GLuint shader : {compile(GL_VERTEX_SHADER, vertex), compile(GL_FRAGMENT_SHADER, fragment)}) {
        glAttachShader(program, shader);
        glDeleteShader(shader);
    }
    glLinkProgram(program);
    check(program, true, "Program link");
    return program;
}
