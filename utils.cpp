#include "utils.h"

RNG g_rng;

float angle(vec2f a, vec2f b) {
    vec2f d = b - a;
    return std::atan2(d.y, d.x);
}
float length(vec2f a){
    return sqrt(a.x * a.x + a.y * a.y);
}
bool VecvAABB(vec2i r1, const AABBi& r2) {
    return (
        r1.x > r2.min.x &&
        r1.x < r2.max.x &&
        r1.y > r2.min.y &&
        r1.y < r2.max.y
           );

}
bool AABBvAABB(const AABBi& r1, const AABBi& r2) {
    return (
        VecvAABB(r1.min, r2) ||
        VecvAABB(r1.max, r2) ||
        VecvAABB(vec2i(r1.min.x, r1.max.y), r2) ||
        VecvAABB(vec2i(r1.min.y, r1.max.x), r2)  ||
        VecvAABB(r2.min, r1) ||
        VecvAABB(r2.max, r1) ||
        VecvAABB(vec2i(r2.min.x, r2.max.y), r1) ||
        VecvAABB(vec2i(r2.min.y, r2.max.x), r1) 
        );
}
bool intersects(const AABBi& a, const AABBi& b) {
    return (a.min.x < b.max.x && a.max.x > b.min.x) &&
             (a.min.y < b.max.y && a.max.y > b.min.y);
}
float sign(float f) {
    return std::copysignf(1.f, f);
}
vec2f sign(vec2f v) {
    return vec2f(sign(v.x), sign(v.y));
}
vec2f normal(vec2f v) {
    return v / length(v);
}
float dot(vec2f a, vec2f b) {
    return a.x * b.x + a.y * b.y;
}
vec2f abs(vec2f v) {
    return vec2f(abs(v.x), abs(v.y));
}
