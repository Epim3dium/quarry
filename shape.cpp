#include "shape.h"

void Shape::add(Rayf ray) {
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
            m_buffer.push_back(vec2i( a.y, a.x ));
        }
        else {
            m_buffer.push_back(vec2i( a.x, a.y ));
        }
        err -= dy;
        if (err < 0) {
            a.y += ystep;
            err += dx;
        }
    }
}
void Shape::add(AABBf aabb) {
    for(int x = aabb.min.x; x < aabb.max.x; x++) {
        m_buffer.push_back(vec2i(x, aabb.min.y));
        m_buffer.push_back(vec2i(x, aabb.max.y));
    }
    for(int y = aabb.min.y; y <= aabb.max.y; y++) {
        m_buffer.push_back(vec2i(aabb.min.x, y));
        m_buffer.push_back(vec2i(aabb.max.x, y));
    }
}

void Shape::draw(Grid& g, clr_t clr) {
    for(auto& v : m_buffer)
        g.drawCellAtClean(v.x, v.y, clr);
    m_buffer.clear();
}
