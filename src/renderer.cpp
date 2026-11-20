#include "renderer.hpp"
#include <array>
#include <vector>

namespace {
struct Vertex { float x,y,z,nx,ny,nz; };
Mesh upload(const std::vector<Vertex>& vertices) {
    Mesh m;
    m.count = static_cast<GLsizei>(vertices.size());
    glGenVertexArrays(1,&m.vao);
    glBindVertexArray(m.vao);
    glGenBuffers(1,&m.buffer);
    glBindBuffer(GL_ARRAY_BUFFER,m.buffer);
    glBufferData(GL_ARRAY_BUFFER,vertices.size()*sizeof(Vertex),vertices.data(),GL_STATIC_DRAW);
    for (unsigned i=0;i<2;++i) {
        glEnableVertexAttribArray(i);
        glVertexAttribPointer(i,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),
                              reinterpret_cast<void*>(i*3*sizeof(float)));
    }
    return m;
}
Mesh cubeMesh() {
    std::vector<Vertex> vertices;
    for (int axis=0;axis<3;++axis) for (int sign : {-1,1}) {
        for (int corner : {0,1,3,0,3,2}) {
            float p[3]{}, n[3]{};
            n[axis]=sign;
            p[axis]=sign*.5f;
            p[(axis+1)%3]=(corner&1) ? .5f : -.5f;
            p[(axis+2)%3]=(corner&2) ? .5f : -.5f;
            vertices.push_back({p[0],p[1],p[2],n[0],n[1],n[2]});
        }
    }
    return upload(vertices);
}
Mesh sphereMesh() {
    std::vector<Vertex> vertices;
    constexpr float pi=3.14159265f;
    for (int row=0;row<20;++row) for (int col=0;col<36;++col) {
        for (int corner : {0,1,3,0,3,2}) {
            float latitude=pi*((row+(corner>>1))/20.f-.5f);
            float longitude=2*pi*(col+(corner&1))/36.f;
            float x=std::cos(latitude)*std::cos(longitude), y=std::sin(latitude);
            float z=std::cos(latitude)*std::sin(longitude);
            vertices.push_back({x,y,z,x,y,z});
        }
    }
    return upload(vertices);
}
}
Renderer::Renderer() : cube(cubeMesh()), sphere(sphereMesh()) {
    program=makeProgram(R"(#version 330 core
layout(location=0) in vec3 pos;
layout(location=1) in vec3 normal;
uniform mat4 model, vp;
out vec3 n, localNormal;
void main(){
    localNormal=normal;
    n=transpose(inverse(mat3(model)))*normal;
    gl_Position=vp*model*vec4(pos,1);
})",R"(#version 330 core
in vec3 n, localNormal;
uniform bool striped;
uniform vec3 tint;
out vec4 frag;
void main(){
    float light=max(dot(normalize(n),normalize(vec3(-.5,.95,.48))),0.);
    vec3 pigment=tint;
    if (striped) {
        float band=abs(dot(normalize(localNormal),normalize(vec3(.5,1,.25))));
        pigment=mix(vec3(1,.84,.56),tint,smoothstep(.09,.15,band));
    }
    frag=vec4(pigment*(.55+.45*light),1);
})");
    striped=glGetUniformLocation(program,"striped");
    model=glGetUniformLocation(program,"model");
    color=glGetUniformLocation(program,"tint");
    viewProjection=glGetUniformLocation(program,"vp");
    glEnable(GL_DEPTH_TEST);
}
Renderer::~Renderer() {
    for (auto m : {cube,sphere}) {
        glDeleteBuffers(1,&m.buffer);
        glDeleteVertexArrays(1,&m.vao);
    }
    glDeleteProgram(program);
}
void Renderer::draw(const Maze& maze, Point marble, const Mat4& board, const Mat4& spin, int w, int h) {
    if (w<=0 || h<=0) return;
    glViewport(0,0,w,h);
    glClearColor(.045f,.078f,.10f,1);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    glUseProgram(program);
    float aspect=float(w)/h;
    float distance=(maze.halfSize+1.3f)/std::min(1.f,aspect);
    Mat4 vp=perspective(.70f,aspect)*camera(Vec3{0,2.8f,1.48f}*distance);
    glUniformMatrix4fv(viewProjection,1,GL_FALSE,data(vp));
    auto drawMesh=[&](Mesh mesh, Mat4 local, Vec3 tint) {
        Mat4 world=board*local;
        glUniformMatrix4fv(model,1,GL_FALSE,data(world));
        glUniform3f(color,tint.x,tint.y,tint.z);
        glBindVertexArray(mesh.vao);
        glDrawArrays(GL_TRIANGLES,0,mesh.count);
    };
    glUniform1i(striped,0);
    auto box=[&](Box b,float bottom,float height,Vec3 tint) {
        drawMesh(cube,translation(Vec3{b.center.x,bottom+height/2,b.center.z})*
                 scaling(Vec3{2*b.half.x,height,2*b.half.z}),tint);
    };
    box({{0,0},{maze.halfSize+.08f,maze.halfSize+.08f}},-.32f,.32f,Vec3{.76f,.64f,.46f});
    box(maze.goal,-.32f,.325f,Vec3{.12f,.78f,.61f});
    box({maze.start,{.48f,.48f}},0,.012f,Vec3{.26f,.54f,.64f});
    for (auto wall : maze.walls) box(wall,0,.42f,Vec3{.19f,.34f,.39f});
    glUniform1i(striped,1);
    drawMesh(sphere,translation(Vec3{marble.x,marbleRadius+.012f,marble.z})*spin*
             scaling(Vec3{marbleRadius,marbleRadius,marbleRadius}),Vec3{1,.39f,.14f});
}
