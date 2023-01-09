#include "include/rigidbody.h"
#include "include/core.h"

bool CircleRigidbody::processRigidbodyCol(CircleRigidbody& other, vec2f& pos, vec2f& other_p) {
    if(!isActive || !other.isActive)
        return false;
    Rayf ray  = {pos, this->vel};
    vec2f closest = getClosestPoint(ray, other_p);
    if(length(other_p - closest) < radius + other.radius) {
        vec2f n = normal(other_p - closest);
        float f = 1.f / length(other_p - pos) / 2.f;
        if(length(n) != 0.f) {
            //add bounce
            vel.x -= (1.f + f) * (n.x); 
            vel.y -= (1.f + f) * (n.y);

            other.vel.x += (1.f + f) * (n.x); 
            other.vel.y += (1.f + f) * (n.y);
        }
        return true;
    }
    return false;

}
void CircleRigidbody::update(Grid& g, vec2f& pos) {
    if(!isActive)
        return;
    auto isCollidable = [&](const CellVar& cv) {
        return cv.getProperty().density > 1000;
    };
    float vl = length(vel);
    vec2f vn = normal(vel);
    //drag
    if(abs(vel.x) > 0.001f || abs(vel.y) > 0.001f)
        vel -= vn * vl * vl * physics.drag;
    //collision
    vec2f col_n = {0, 0};
    for(size_t i = 0; i < physics.raycast_c; i++) {
        float ang = ((float)i / physics.raycast_c) * 2.f * 3.141f;
        vec2f dir = vec2f(sinf(ang), cosf(ang));
        vec2f point = pos + dir * radius;
        if(isCollidable(g.get(point.x, point.y)))
            col_n -= dir;
    }
    //resolve collision
    if(length(col_n) != 0.f) {
        col_n = normal(col_n);
        float d = std::clamp(dot(vel, col_n), -INFINITY, 0.f);

        float a = atan2(col_n.x, col_n.y);
        a -= atan2(vn.x, vn.y);
        a = abs(a);

        auto remap = Remap<float, float>(0.f, 1.f, 1.f, 50.f);

        //add bounce
        vel.x -= (1.f + physics.bounciness) * (col_n.x * d); 
        vel.y -= (1.f + physics.bounciness) * (col_n.y * d);

        //add friciton
        vel *= 1.f - physics.friction * abs(sin(a));
    }
    if(g.get((vec2i)pos).getProperty().state == eState::Liquid) {
        vel.y += (g.get((vec2i)pos).getProperty().density - physics.density) / 1000.f;
        vel.x *= 1.f - (g.get((vec2i)pos).getProperty().density - physics.density) / 1000.f;
    }
    pos += vel;
}
