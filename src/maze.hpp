#pragma once
#include <vector>

struct Point { float x=0, z=0; };
inline Point operator+(Point a, Point b) { return {a.x+b.x,a.z+b.z}; }
inline Point operator-(Point a, Point b) { return {a.x-b.x,a.z-b.z}; }
inline Point operator*(Point a, float k) { return {a.x*k,a.z*k}; }
inline float dot(Point a, Point b) { return a.x*b.x+a.z*b.z; }

// Horizontal footprint. Walls share a constant height from the floor.
struct Box { Point center, half; };
struct Maze {
    float halfSize;
    Point start, opening, outward;
    Box goal;
    std::vector<Box> walls;
};
constexpr float marbleRadius=.24f, wallWidth=.16f;
Maze fixedMaze();
