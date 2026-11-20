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
int main() {
    try { motionChecks(); }
    catch(const std::exception& e) { std::cerr<<e.what()<<'\n'; return 1; }
}
