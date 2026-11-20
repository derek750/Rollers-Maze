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
