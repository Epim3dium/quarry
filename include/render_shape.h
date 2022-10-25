#pragma once
#include "core.h"
#include "utils.h"

struct PxDrawInfo {
    vec2i pos;
    clr_t clr;
};
class Shape {
    std::vector<PxDrawInfo> m_buffer;
    public:
    void add(Rayf ray, clr_t clr);
    void add(AABBf aabb, clr_t clr);

    void draw(Grid& g);

    Shape() {}
};
