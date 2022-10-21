#pragma once
#include "utils.h"
#include <iostream>

class Grid;

class CircleRigidbody {
public:
    struct {
        float bounciness = 0.1f;
        float drag = 0.03f;
        size_t raycast_c = 16U;
        float friction = 0.1f;
        int density = 900;
    }physics;
    float radius = 5.0f;
    vec2f vel = {0, 0};

    void update(Grid& g, vec2f& pos);
    bool processRigidbodyCol(CircleRigidbody& rb, vec2f& m_pos, vec2f& o_pos);
};
