#pragma once
#include "utils.h"

class QuadTree {
    public:
    AABBi AABB;
    //top left; top right; low left; low right
    QuadTree* parent;
    std::unique_ptr<QuadTree> roots[4];

    inline vec2i getMin() const { 
        return AABB.min;
    }
    inline vec2i getMax() const { 
        return AABB.max;
    }
    inline bool isSplit() const {
        return roots[0] != nullptr;
    }
    void split();
    void splitTill(const vec2i& size_smallest);
    
    QuadTree* getContaining(vec2i pos);

    QuadTree(vec2i min, vec2i max, QuadTree* parent_ = nullptr) : parent(parent_) {
        AABB.min = min;
        AABB.max = max;
        for(int i = 0; i < 4; i++)
            roots[i] = nullptr;
    }
    QuadTree(vec2i size_, QuadTree* parent_ = nullptr) : parent(parent_) {
        AABB.min = {0, 0};
        AABB.max = size_;
        for(int i = 0; i < 4; i++)
            roots[i] = nullptr;
    }
};
