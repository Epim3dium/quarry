#include "quad_tree.h"

QuadTree* QuadTree::getContaining(vec2i pos) {
    if(!isSplit())
        return nullptr;
    for(int i = 0; i < 4; i++) {
        if(roots[i]->AABB.contains(pos))
            return roots[i].get();
    }
    return nullptr;
}
void QuadTree::split() {
    if(isSplit())
        return;
    vec2i min = AABB.min;
    vec2i max = AABB.max;
    vec2i half = AABB.min + (AABB.max - AABB.min) / 2;
    roots[0] = std::make_unique<QuadTree>(QuadTree({min.x, half.y}, {half.x, max.y}, this));
    roots[1] = std::make_unique<QuadTree>(QuadTree(half, max, this));
    roots[2] = std::make_unique<QuadTree>(QuadTree(min, half, this));
    roots[3] = std::make_unique<QuadTree>(QuadTree({half.x, min.y}, {max.x, half.y}, this));
}
void QuadTree::splitTill(const vec2i& size_smallest) {
    if(AABB.size().x / 2 <= size_smallest.x || AABB.size().y / 2 <= size_smallest.y)
        return;
    split();
    for(auto& r : roots) {
        r->splitTill(size_smallest);
    }
}
