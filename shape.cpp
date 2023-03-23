
#include "shape.h"

namespace epi {
void Shape::add(Ray ray) {
    vec2f a = ray.pos;
    vec2f b = ray.pos + ray.dir;
    int steep = abs(b.y - a.y) > abs(b.x - a.x);
    if (steep) {
        std::swap(a.x, a.y);
        std::swap(b.x, b.y);
    }
    if (a.x > b.x) {
        std::swap(a.x, b.x);
        std::swap(a.y, b.y);
    }
    int dx, dy;
    dx = b.x - a.x;
    dy = abs(b.y - a.y);
    int err = dx / 2;
    int ystep;
    if (a.y < b.y) {
        ystep = 1;
    }
    else {
        ystep = -1;
    }
    for (; a.x <= b.x; a.x++) {
        if (steep) {
            m_buffer.insert(convert(vec2i( a.y, a.x )));
        }
        else {
            m_buffer.insert(convert(vec2i( a.x, a.y )));
        }
        err -= dy;
        if (err < 0) {
            a.y += ystep;
            err += dx;
        }
    }
}
void Shape::add(AABB aabb) {
    for(int x = aabb.min.x; x < aabb.max.x; x++) {
        m_buffer.insert(convert(vec2i(x, aabb.min.y)));
        m_buffer.insert(convert(vec2i(x, aabb.max.y)));
    }
    for(int y = aabb.min.y; y < aabb.max.y; y++) {
        m_buffer.insert(convert(vec2i(aabb.min.x, y)));
        m_buffer.insert(convert(vec2i(aabb.max.x, y)));
    }
}

}
