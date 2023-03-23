
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
            add(vec2i( a.y, a.x ));
        }
        else {
            add(vec2i( a.x, a.y ));
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
        add(vec2i(x, aabb.min.y));
        add(vec2i(x, aabb.max.y));
    }
    for(int y = aabb.min.y; y < aabb.max.y; y++) {
        add(vec2i(aabb.min.x, y));
        add(vec2i(aabb.max.x, y));
    }
}
void addCircleHelper(int xc, int yc, int x, int y, Shape& s)
{
    s.add({xc+x, yc+y});
    s.add({xc-x, yc+y});
    s.add({xc+x, yc-y});
    s.add({xc-x, yc-y});
    s.add({xc+y, yc+x});
    s.add({xc-y, yc+x});
    s.add({xc+y, yc-x});
    s.add({xc-y, yc-x});
}
void Shape::add(epi::Circle circ) {
    int xc = circ.pos.x;
    int yc = circ.pos.y;
    int r = circ.radius;
    int x = 0, y = r;
    int d = 3 - 2 * r;
    addCircleHelper(xc, yc, x, y, *this);
    while (y >= x)
    {
        // for each pixel we will
        // draw all eight pixels
         
        x++;
 
        // check for decision parameter
        // and correspondingly
        // update d, x, y
        if (d > 0)
        {
            y--;
            d = d + 4 * (x - y) + 10;
        }
        else
            d = d + 4 * x + 6;
        addCircleHelper(xc, yc, x, y, *this);
    }
}

}
