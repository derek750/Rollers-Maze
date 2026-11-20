#include "physics.hpp"

Mat4 boardRotation(Tilt tilt) {
    return rotation(tilt.roll,Vec3{0,0,1})*rotation(tilt.pitch,Vec3{1,0,0});
}
Point downhill(const Mat4& board) {
    // Inverse of a pure rotation is its transpose. Dot world gravity
    // (0,-24,0) with the board's local X and Z basis vectors.
    const float* m=data(board);
    return {-24.f*m[1],-24.f*m[9]};
}
void advance(Marble& ball,Point acceleration,float dt) {
    ball.velocity=(ball.velocity+acceleration*dt)*std::exp(-.7f*dt);
    float speed=std::sqrt(dot(ball.velocity,ball.velocity));
    if (speed>maxSpeed) ball.velocity=ball.velocity*(maxSpeed/speed);
    ball.position=ball.position+ball.velocity*dt;
}

bool resolveContact(Marble& ball,const Box& wall) {
    Point offset=ball.position-wall.center;
    Point closest{std::clamp(offset.x,-wall.half.x,wall.half.x),
                  std::clamp(offset.z,-wall.half.z,wall.half.z)};
    Point delta=offset-closest;
    float squared=dot(delta,delta);
    if (squared>=marbleRadius*marbleRadius) return false;
    Point normal;
    float depth;
    if (squared>0) {
        float distance=std::sqrt(squared);
        normal=delta*(1/distance);
        depth=marbleRadius-distance;
    } else {
        float dx=wall.half.x-std::abs(offset.x), dz=wall.half.z-std::abs(offset.z);
        normal=dx<dz ? Point{std::copysign(1.f,offset.x),0} : Point{0,std::copysign(1.f,offset.z)};
        depth=marbleRadius+std::min(dx,dz);
    }
    ball.position=ball.position+normal*depth;
    float inward=std::min(0.f,dot(ball.velocity,normal));
    ball.velocity=ball.velocity-normal*inward;
    return true;
}
void stepMaze(const Maze& maze,Marble& ball,Point acceleration,float dt,ExitState& exit) {
    int count=std::max(1,int(std::ceil(maxSpeed*dt/(wallWidth*.5f))));
    for (int i=0;i<count && !exit.won;++i) {
        Point before=ball.position;
        advance(ball,acceleration,dt/count);
        for (int pass=0;pass<4;++pass) {
            bool contact=false;
            for (const auto& wall : maze.walls) contact|=resolveContact(ball,wall);
            if (!contact) break;
        }
        float oldSide=dot(before-maze.opening,maze.outward);
        float side=dot(ball.position-maze.opening,maze.outward);
        Point offset=ball.position-maze.goal.center;
        bool onPad=std::abs(offset.x)<=maze.goal.half.x && std::abs(offset.z)<=maze.goal.half.z;
        if (side<=0) exit.crossed=false;
        if (oldSide<=0 && side>0 && onPad) exit.crossed=true;
        exit.won=exit.crossed && onPad && side>=marbleRadius;
        if (exit.won) ball.velocity={};
    }
}
