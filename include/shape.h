#pragma once
#include "grid.hpp"
#include "utils.h"
#include "types.hpp"
#include <set>

namespace epi {
class Shape {
    std::set<long long int> m_buffer;
    static long long int convert(vec2i v) {
        return *reinterpret_cast<long long int*>(&v);
    }
    static vec2i convert(long long int i) {
        return *reinterpret_cast<vec2i*>(&i);
    }
    public:
    std::vector<vec2i> getPoints() const { 
        std::vector<vec2i> r;
        for(auto i : m_buffer)
            r.push_back(convert(i));
        return r;
    }

    inline void add(vec2i v) {
        m_buffer.insert(convert(v));
    }
    void add(epi::Ray ray);
    void add(epi::AABB aabb);
    void add(epi::Circle circ);

    Shape() {}
};
}
