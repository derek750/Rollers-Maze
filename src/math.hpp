#pragma once
#include <algorithm>
#include <cmath>

// Column vectors throughout: parent * child applies child first.
#ifdef ROLLERS_APPLE_SIMD
#include <simd/simd.h>
using Mat4 = simd_float4x4;
using Vec3 = simd_float3;
inline Mat4 operator*(const Mat4& a,const Mat4& b) { return simd_mul(a,b); }
inline Mat4 identity() { return matrix_identity_float4x4; }
inline Mat4 translation(Vec3 v) {
    Mat4 m = identity();
    m.columns[3] = {v.x,v.y,v.z,1};
    return m;
}
inline Mat4 scaling(Vec3 v) {
    Mat4 m = identity();
    m.columns[0].x=v.x; m.columns[1].y=v.y; m.columns[2].z=v.z;
    return m;
}
inline Mat4 rotation(float angle, Vec3 axis) {
    return simd_matrix4x4(simd_quaternion(angle, simd_normalize(axis)));
}
inline Mat4 camera(Vec3 eye) {
    Vec3 back = simd_normalize(eye);
    Vec3 right = simd_normalize(simd_cross(Vec3{0,1,0},back));
    Vec3 up = simd_cross(back,right);
    Mat4 axes = simd_matrix(simd_make_float4(right,0),simd_make_float4(up,0),
                            simd_make_float4(back,0),simd_make_float4(0,0,0,1));
    return simd_transpose(axes) * translation(-eye);
}
inline Mat4 perspective(float fov, float aspect) {
    float f = 1/std::tan(fov/2), near = .1f, far = 100;
    return simd_matrix(simd_make_float4(f/aspect,0,0,0),simd_make_float4(0,f,0,0),
                       simd_make_float4(0,0,(far+near)/(near-far),-1),
                       simd_make_float4(0,0,2*far*near/(near-far),0));
}
inline const float* data(const Mat4& m) { return reinterpret_cast<const float*>(&m); }
#else
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
using Mat4 = glm::mat4;
using Vec3 = glm::vec3;
inline Mat4 identity() { return Mat4(1); }
inline Mat4 translation(Vec3 v) { return glm::translate(identity(),v); }
inline Mat4 scaling(Vec3 v) { return glm::scale(identity(),v); }
inline Mat4 rotation(float angle, Vec3 axis) { return glm::rotate(identity(),angle,axis); }
inline Mat4 camera(Vec3 eye) { return glm::lookAt(eye,Vec3(0),Vec3(0,1,0)); }
inline Mat4 perspective(float fov,float aspect) { return glm::perspective(fov,aspect,.1f,100.f); }
inline const float* data(const Mat4& m) { return glm::value_ptr(m); }
#endif
