#include "maze.hpp"

Maze fixedMaze() {
    Maze m{3, {-2,2}, {2,-3}, {0,-1}, {{2,-3.6f},{1,.6f}}, {}};
    m.walls = {
        {{-3,0},{.08f,3.08f}}, {{3,0},{.08f,3.08f}},
        {{0,3},{3,.08f}}, {{-1,-3},{2,.08f}},
        {{-1,1},{.08f,2}}, {{1,-1},{.08f,2}}
    };
    return m;
}

#include <algorithm>
#include <queue>
#include <random>

namespace {
constexpr int dx[]={0,1,0,-1}, dz[]={-1,0,1,0};
int opposite(int d) { return (d+2)%4; }
void carve(Maze& m,std::mt19937& rng) {
    m.cells.fill(15);
    std::array<bool,cellCount> visited{};
    std::vector<int> stack{m.startCell};
    visited[m.startCell]=true;
    while (!stack.empty()) {
        int cell=stack.back(), count=0;
        std::array<int,4> choices{};
        for (int d=0;d<4;++d) {
            int next=neighbor(cell,d);
            if (next>=0 && !visited[next]) choices[count++]=d;
        }
        if (!count) { stack.pop_back(); continue; }
        int d=choices[std::uniform_int_distribution<int>(0,count-1)(rng)];
        int next=neighbor(cell,d);
        m.cells[cell]&=~(1<<d);
        m.cells[next]&=~(1<<opposite(d));
        visited[next]=true;
        stack.push_back(next);
    }
}
int chooseExit(Maze& m) {
    std::array<int,cellCount> distance;
    distance.fill(-1);
    distance[m.startCell]=0;
    std::queue<int> queue;
    queue.push(m.startCell);
    int best=-1, direction=north;
    while (!queue.empty()) {
        int cell=queue.front(); queue.pop();
        for (int d=0;d<4;++d) {
            int next=neighbor(cell,d);
            if (next<0 && distance[cell]>best) {
                best=distance[cell]; m.exitCell=cell; direction=d;
            }
            if (next<0 || (m.cells[cell]&(1<<d)) || distance[next]>=0) continue;
            distance[next]=distance[cell]+1;
            queue.push(next);
        }
    }
    return direction;
}
}
int neighbor(int cell,int d) {
    int x=cell%gridSize+dx[d], z=cell/gridSize+dz[d];
    if (x<0 || z<0 || x>=gridSize || z>=gridSize) return -1;
    return z*gridSize+x;
}
Point cellCenter(int cell) {
    return {(cell%gridSize+.5f-gridSize*.5f)*cellWidth,
            (cell/gridSize+.5f-gridSize*.5f)*cellWidth};
}
Maze generateMaze(std::uint32_t seed) {
    std::mt19937 rng(seed);
    Maze m{};
    m.halfSize=gridSize*cellWidth/2;
    m.startCell=std::uniform_int_distribution<int>(0,cellCount-1)(rng);
    m.start=cellCenter(m.startCell);
    carve(m,rng);
    int exitDirection=chooseExit(m);
    m.cells[m.exitCell]&=~(1<<exitDirection);
    m.outward={float(dx[exitDirection]),float(dz[exitDirection])};
    float half=cellWidth/2, thin=wallWidth/2;
    m.opening=cellCenter(m.exitCell)+m.outward*half;
    m.goal={m.opening+m.outward*half,{half,half}};
    for (int cell=0;cell<cellCount;++cell) for (int d=0;d<4;++d) {
        if (!(m.cells[cell]&(1<<d))) continue;
        if ((d==west || d==north) && neighbor(cell,d)>=0) continue;
        Point center=cellCenter(cell)+Point{float(dx[d]),float(dz[d])}*half;
        Point extent=dx[d] ? Point{thin,half+thin} : Point{half+thin,thin};
        m.walls.push_back({center,extent});
    }
    // Guard the sides of the landing until the marble clears the threshold.
    Point tangent{-m.outward.z,m.outward.x};
    Point rail=m.outward.x ? Point{half,thin} : Point{thin,half};
    for (float sign : {-1.f,1.f}) m.walls.push_back({m.goal.center+tangent*(sign*half),rail});
    return m;
}
