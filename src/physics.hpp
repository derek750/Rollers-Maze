#pragma once
#include "math.hpp"
#include "maze.hpp"

constexpr double fixedStep=1.0/120.0;
constexpr float maxTilt=.2617994f, maxSpeed=6.f;
struct Tilt { float pitch=0, roll=0; };
Mat4 boardRotation(Tilt tilt);
Point downhill(const Mat4& board);

struct Marble { Point position, velocity; };
void advance(Marble& ball, Point acceleration, float dt);

struct Clock {
    double pending=0;
    template<class Step> void update(double elapsed, Step step) {
        pending+=std::clamp(elapsed,0.0,.1);
        while (pending+1e-12>=fixedStep) {
            step(static_cast<float>(fixedStep));
            pending-=fixedStep;
        }
    }
};
