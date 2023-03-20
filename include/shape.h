#pragma once
#include "grid.hpp"
#include "utils.h"

class Shape {
    std::vector<vec2i> m_buffer;
    public:
    inline const std::vector<vec2i>& getPoints() const { return m_buffer; }

    inline void add(vec2i v) {
        m_buffer.push_back(v);
    }
    void add(Rayf ray);
    void add(AABBf aabb);

    void draw(Grid& g, clr_t clr);

    Shape() {}
};
