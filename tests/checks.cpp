#include "physics.hpp"
#include <stdexcept>
#include <iostream>

void check(bool pass,const char* description) {
    if (!pass) throw std::runtime_error(description);
}
void motionChecks() {
    Marble ball{};
    advance(ball,downhill(boardRotation({})),fixedStep);
    check(dot(ball.position,ball.position)==0,"level board moved from rest");
    for (float sign : {-1.f,1.f}) {
        check(sign*downhill(boardRotation({sign*maxTilt,0})).z>0,"pitch downhill");
        check(sign*downhill(boardRotation({0,sign*maxTilt})).x<0,"roll downhill");
    }
    auto simulate=[](int hz) {
        Clock clock;
        Marble b{};
        for (int i=0;i<hz;++i)
            clock.update(1./hz,[&](float dt){advance(b,{1,2},dt);});
        return b.position;
    };
    for (int hz : {60,144}) {
        Point error=simulate(hz)-simulate(30);
        check(dot(error,error)<1e-10f,"fixed step depends on frame rate");
    }
}
void contactChecks() {
    Box wall{{0,0},{.08f,2}};
    Marble hit{{-.2f,0},{4,1}};
    check(resolveContact(hit,wall),"expected wall contact");
    check(hit.position.x<=-.3199f && hit.velocity.x==0 && hit.velocity.z==1,"wall slide");
    Marble embedded{{.02f,0},{}};
    resolveContact(embedded,wall);
    check(embedded.position.x>=.3199f,"nearest-face separation");
    Maze maze=fixedMaze();
    ExitState exit;
    Marble corner{{-2,-2},{-5,-5}};
    for (int i=0;i<120;++i) stepMaze(maze,corner,{-5,-5},fixedStep,exit);
    check(std::abs(corner.position.x+2.68f)<.001f && std::abs(corner.position.z+2.68f)<.001f,"inside corner");
    Marble fast{{-2.5f,0},{-maxSpeed,0}};
    stepMaze(maze,fast,{-6,0},.5f,exit);
    check(fast.position.x>=-2.681f && fast.velocity.x==0 && !exit.won,"fast boundary impact");
    Marble goal{{2,-2.8f},{0,-4}};
    stepMaze(maze,goal,{},.25f,exit);
    check(exit.won,"exit crossing");
    exit={};
    goal={maze.goal.center,{}};
    stepMaze(maze,goal,{},fixedStep,exit);
    check(!exit.won,"goal without crossing");
}
int main() {
    try { motionChecks(); contactChecks(); }
    catch(const std::exception& e) { std::cerr<<e.what()<<'\n'; return 1; }
}
