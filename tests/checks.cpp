#include "physics.hpp"
#include <stdexcept>
#include <iostream>
#include <queue>

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
void generationChecks() {
    for (unsigned seed=0;seed<1024;++seed) {
        Maze m=generateMaze(seed), replay=generateMaze(seed);
        check(m.startCell==replay.startCell && m.cells==replay.cells,"seed replay");
        std::array<int,cellCount> depth{};
        depth.fill(-1);
        std::queue<int> q;
        q.push(m.startCell); depth[m.startCell]=0;
        int reached=0, gates=0, farthest=0;
        while (!q.empty()) {
            int cell=q.front(); q.pop(); ++reached;
            for (int d=0;d<4;++d) {
                int next=neighbor(cell,d);
                bool closed=m.cells[cell]&(1<<d);
                if (next<0) {
                    farthest=std::max(farthest,depth[cell]);
                    if (!closed) { ++gates; check(cell==m.exitCell,"exit location"); }
                    continue;
                }
                bool shared=m.cells[next]&(1<<((d+2)%4));
                check(closed==shared,"shared edge mismatch");
                if (closed || depth[next]>=0) continue;
                depth[next]=depth[cell]+1; q.push(next);
            }
        }
        check(reached==cellCount && gates==1 && depth[m.exitCell]==farthest,"maze topology");
    }
}
// Follow cell centers using only legal tilts and the production simulation.
void playThrough(unsigned seed) {
    Maze maze=generateMaze(seed);
    std::array<int,cellCount> parent;
    parent.fill(-1);
    std::queue<int> q;
    q.push(maze.startCell);
    parent[maze.startCell]=maze.startCell;
    while (!q.empty()) {
        int cell=q.front(); q.pop();
        for (int d=0;d<4;++d) {
            int next=neighbor(cell,d);
            if (next<0 || parent[next]>=0 || (maze.cells[cell]&(1<<d))) continue;
            parent[next]=cell; q.push(next);
        }
    }
    std::vector<Point> route;
    for (int c=maze.exitCell;c!=maze.startCell;c=parent[c]) route.push_back(cellCenter(c));
    std::reverse(route.begin(),route.end());
    route.push_back(maze.goal.center);
    Marble ball{maze.start,{}};
    ExitState exit;
    for (Point target : route) {
        bool arrived=false;
        for (int tick=0;tick<1200 && !arrived && !exit.won;++tick) {
            Point error=target-ball.position;
            Point drive=error*8-ball.velocity*5;
            Tilt tilt{std::clamp(drive.z/24,-maxTilt,maxTilt),
                      std::clamp(-drive.x/24,-maxTilt,maxTilt)};
            stepMaze(maze,ball,downhill(boardRotation(tilt)),fixedStep,exit);
            arrived=dot(error,error)<.0025f;
        }
        check(arrived || exit.won,"route blocked by physical geometry");
    }
    check(exit.won,"physical playthrough did not finish");
}
int main() {
    try { motionChecks(); contactChecks(); generationChecks(); for (unsigned seed : {0u,42u,197u,2026u}) playThrough(seed); }
    catch(const std::exception& e) { std::cerr<<e.what()<<'\n'; return 1; }
}
