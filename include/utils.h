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


template <class T>
struct AABB {
    sf::Vector2<T> min;
    sf::Vector2<T> max;
    sf::Vector2<T> size() const {
        return max - min;
    }
    T area() const {
        auto t = size();
        return t.x * t.y;
    }
    sf::Vector2<T> center() const {
        return min + (max - min) / static_cast<T>(2);
    }
    bool contains(sf::Vector2<T> v) {
        return v.x >= min.x && v.x < max.x &&
            v.y >= min.y && v.y < max.y;
    }
    float distance(T x, T y) {
      float dx = std::max(abs(min.x - x), abs(x - max.x));
      float dy = std::max(abs(min.y - y), abs(y - max.y));
      return sqrt(dx*dx + dy*dy);
    }
    void update_to_contain(const sf::Vector2<T>& v) {
        min.x = std::min(min.x, v.x);
        min.y = std::min(min.y, v.y);

        max.x = std::max(max.x, v.x);
        max.y = std::max(max.y, v.y);
    }
    AABB() {}
    AABB(sf::Vector2<T> min_, sf::Vector2<T> max_) : min(min_), max(max_)  {}
};
template <class T>
struct Ray {
    sf::Vector2<T> pos;
    sf::Vector2<T> dir;
};

typedef Ray<int> Rayi;
typedef Ray<float> Rayf;

typedef AABB<int> AABBi;
typedef AABB<float> AABBf;

bool RayVAABB(vec2f ray_origin, vec2f ray_dir,
    const AABBf& target, float* t_hit_near,
    vec2f* contact_normal, vec2f* contact_point);
bool AABBvAABB(const AABBi& r1, const AABBi& r2);
bool VecvAABB(vec2i r1, const AABBi& r2);

bool intersects(const AABBi& a, const AABBi& b);

//calculates angle between 2 vectors
float angle(vec2f a, vec2f b);

float length(vec2f a);

float sign(float f);

vec2f sign(vec2f v);

vec2f normal(vec2f v);

float dot(vec2f a, vec2f b);

vec2f abs(vec2f v);
