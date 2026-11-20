#pragma once
#include <vector>
#include <array>
#include <cstdint>

struct Point { float x=0, z=0; };
inline Point operator+(Point a, Point b) { return {a.x+b.x,a.z+b.z}; }
inline Point operator-(Point a, Point b) { return {a.x-b.x,a.z-b.z}; }
inline Point operator*(Point a, float k) { return {a.x*k,a.z*k}; }
inline float dot(Point a, Point b) { return a.x*b.x+a.z*b.z; }

// Horizontal footprint. Walls share a constant height from the floor.
struct Box { Point center, half; };
constexpr int gridSize=8, cellCount=gridSize*gridSize;
constexpr float cellWidth=1.5f;
enum Direction { north,east,south,west };
int neighbor(int cell,int direction);
Point cellCenter(int cell);
struct Maze {
    float halfSize;
    Point start, opening, outward;
    Box goal;
    std::vector<Box> walls;
    std::array<unsigned char,cellCount> cells{};
    int startCell=0, exitCell=0;
};
constexpr float marbleRadius=.24f, wallWidth=.16f;
Maze fixedMaze();

Maze generateMaze(std::uint32_t seed);
