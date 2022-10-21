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
bool RayVAABB(vec2f ray_origin, vec2f ray_dir,
    const AABBf& target, float* t_hit_near,
    vec2f* contact_normal, vec2f* contact_point)
{
    vec2f invdir = { 1.0f / ray_dir.x, 1.0f / ray_dir.y };
    vec2f t_size = target.size();
    //VVVVVVVVVVVVV
    //if((int)target.size.y % 2 == 0 && target.pos.y > ray_origin.y)
    //t_size -= vec2f(0, 1);
    //^^^^^^^^^^^^^
    vec2f t_near = (target.center() - t_size / 2.f - ray_origin);
    t_near.x *= invdir.x;
    t_near.y *= invdir.y;
    vec2f t_far = (target.center() + t_size / 2.f - ray_origin);
    t_far.x *= invdir.x;
    t_far.y *= invdir.y;

    if (std::isnan(t_far.y) || std::isnan(t_far.x)) return false;
    if (std::isnan(t_near.y) || std::isnan(t_near.x)) return false;
    if (t_near.x > t_far.x) std::swap(t_near.x, t_far.x);
    if (t_near.y > t_far.y) std::swap(t_near.y, t_far.y);

    if (t_near.x > t_far.y || t_near.y > t_far.x) return false;
    float thn = std::max(t_near.x, t_near.y);
    if (t_hit_near)
        *t_hit_near = thn;
    float t_hit_far = std::min(t_far.x, t_far.y);

    if (t_hit_far < 0)
        return false;
    if(contact_point)
        *contact_point = ray_origin + ray_dir * thn;
    if (t_near.x > t_near.y && contact_normal) {
        if (invdir.x < 0) {
            *contact_normal = { 1, 0 };
        }
        else {
            *contact_normal = { -1, 0 };
        }
    } 
    else if (t_near.x < t_near.y && contact_normal) {
        if (invdir.y < 0) {
            *contact_normal = { 0, 1 };
        } 
        else {
            *contact_normal = { 0, -1 };
        }
    }
    return true;
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
