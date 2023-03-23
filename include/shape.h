#pragma once
#include "grid.hpp"
#include "utils.h"
#include "types.hpp"
#include <set>

namespace epi {
class Shape {
    static long long int convert(vec2i v) {
        return *reinterpret_cast<long long int*>(&v);
    }
    static vec2i convert(long long int i) {
        return *reinterpret_cast<vec2i*>(&i);
    }
    struct VecCmp {
        bool operator()(const vec2i& lhs, const vec2i& rhs) const { 
            return Shape::convert(lhs) < Shape::convert(rhs); 
        }
    };
    std::set<vec2i, VecCmp> m_buffer;
public:
    std::vector<vec2i> getPoints() const { 
        std::vector<vec2i> r(m_buffer.begin(), m_buffer.end());
        return std::move(r);
    }

    inline void add(vec2i v) {
        m_buffer.insert(v);
    }
    void add(epi::Ray ray);
    void add(epi::AABB aabb);
    void add(epi::Circle circ);
    friend void addCircleHelper(int xc, int yc, int x, int y, Shape& s);
    Shape() {}
};
}
