#pragma once
#include <math.h>

#include <SFML/Graphics.hpp>

#include "RNG.h"
#include "remap.h"
#include "timer.h"

extern RNG g_rng;

typedef sf::Vector2f vec2f;
typedef sf::Vector2i vec2i;
typedef sf::Vector2u vec2u;
typedef sf::Vector2<bool> vec2b;
typedef sf::Color clr_t;
typedef sf::RenderWindow window_t;


struct AABBi {
    vec2i min;
    vec2i max;
    vec2i size() const {
        return max - min;
    }
    int area() {
        auto t = size();
        return t.x * t.y;
    }
    bool contains(vec2i v) {
        return v.x >= min.x && v.x < max.x &&
            v.y >= min.y && v.y < max.y;
    }
    float distance(int x, int y) {
      float dx = std::max(abs(min.x - x), abs(x - max.x));
      float dy = std::max(abs(min.y - y), abs(y - max.y));
      return sqrt(dx*dx + dy*dy);
    }
    void update_to_contain(const vec2i& v) {
        min.x = std::min(min.x, v.x);
        min.y = std::min(min.y, v.y);

        max.x = std::max(max.x, v.x);
        max.y = std::max(max.y, v.y);
    }
};

bool intersects(const AABBi& a, const AABBi& b);

//calculates angle between 2 vectors
float angle(vec2f a, vec2f b);

float length(vec2f a);
