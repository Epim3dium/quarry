#pragma once
#include <cmath>
#include <iostream>
#include <math.h>
#include <vector>

#include "SFML/Graphics/RenderWindow.hpp"
#include "imgui.h"
#include "imgui-SFML.h"

#include "SFML/Graphics.hpp"

#include "transform.hpp"

#define CONSOLAS_PATH "assets/Consolas.ttf"
#define EPI_NAMESPACE epi

namespace EPI_NAMESPACE {

typedef sf::Vector2f vec2f;
typedef sf::Vector2u vec2u;
typedef sf::Vector2i vec2i;
typedef sf::Color Color;
typedef sf::RenderWindow Window;

namespace PastelColor {
    const Color bg =     Color(0xa89984ff);
    const Color bg1 =    Color(0xa89984ff);
    const Color bg2 =    Color(0xbdae93ff);
    const Color bg3 =    Color(0xebdbb2ff);
    const Color bg4 =    Color(0x1e3a4cff);
    const Color Red =    Color(0x9d0006ff);
    const Color Green =  Color(0x79740eff);
    const Color Yellow = Color(0xb57614ff);
    const Color Blue =   Color(0x076678ff);
    const Color Purple = Color(0x8f3f71ff);
    const Color Aqua =   Color(0x427b58ff);
    const Color Orange = Color(0xaf3a03ff);
    const Color Gray =   Color(0x928374ff);
};

float len(vec2f);
vec2f norm(vec2f);
float qlen(vec2f);
float dot(vec2f, vec2f);
float cross(vec2f, vec2f);
vec2f sign(vec2f);

struct Circle;
struct Polygon;

struct AABB {
    vec2f min;
    vec2f max;

    vec2f bl() const {
        return min;
    }
    vec2f br() const {
        return vec2f(max.x, min.y);
    }
    vec2f tl() const {
        return vec2f(min.x, max.y);
    }
    vec2f tr() const {
        return max;
    }

    vec2f center() const {
        return min + (max-min) / 2.f;
    }
    float right() const {
        return max.x;
    };
    float left() const {
        return min.x;
    };
    float bot() const {
        return max.y;
    };
    float top() const {
        return min.y;
    };
    vec2f size() const {
        return max - min;
    }
    void setCenter(vec2f c) {
        auto t = size();
        min = c - t / 2.f;
        max = c + t / 2.f;
    }
    void setSize(vec2f s) {
        auto t = center();
        min = t - s / 2.f;
        max = t + s / 2.f;
    }
    static AABB CreateMinMax(vec2f min, vec2f max);
    static AABB CreateCenterSize(vec2f center, vec2f size);
    static AABB CreateMinSize(vec2f min, vec2f size);
    static AABB CreateFromCircle(const Circle& c);
    static AABB CreateFromPolygon(const Polygon& p);
};
struct Ray {
    vec2f pos;
    vec2f dir;

    float length() const {
        return len(dir);
    }
    Ray() {}
    static Ray CreatePoints(vec2f a, vec2f b);
    static Ray CreatePositionDirection(vec2f p, vec2f d);
};
struct Circle {

    vec2f pos;
    float radius;
    Circle(vec2f p = vec2f(0, 0), float r = 1.f) : pos(p), radius(r) {}
};
class Polygon {
    std::vector<vec2f> points;
    std::vector<vec2f> model;
    float rotation;
    vec2f pos;
    void m_updatePoints() {
        for(size_t i = 0; i < model.size(); i++) {
            const auto& t = model[i];
            points[i].x = t.x * cosf(rotation) - t.y * sinf(rotation);
            points[i].y = t.x * sinf(rotation) + t.y * cosf(rotation);
            points[i] += pos;
        }
    }
    void m_avgPoints() {
        vec2f avg = vec2f(0, 0);
        for(auto& p : model) {
            avg += p;
        }
        avg /= (float)model.size();
        for(auto& p : model) {
            p -= avg;
        }
    }
public:
    float getRot() const {
        return rotation;
    }
    void setRot(float r) {
        rotation = r;
        m_updatePoints();
    }
    vec2f getPos() const {
        return pos;
    }
    void setPos(vec2f v) {
        pos = v;
        m_updatePoints();
    }
    void scale(vec2f s) {
        for(auto& m : model) {
            m.x *= s.x;
            m.y *= s.y;
        }
        m_updatePoints();
    }

    const std::vector<vec2f>& getVertecies() const {
        return points;
    }
    const std::vector<vec2f>& getModelVertecies() const {
        return model;
    }
    Polygon() {}
    Polygon(vec2f pos_, float rot_, const std::vector<vec2f>& model_) : pos(pos_), rotation(rot_), model(model_), points(model_.size(), vec2f(0, 0)) {
        std::sort(model.begin(), model.end(), [](vec2f a, vec2f b) {
                      auto anga = std::atan2(a.x, a.y);
                      if (anga > M_PI)        { anga -= 2 * M_PI; }
                      else if (anga <= -M_PI) { anga += 2 * M_PI; }
                      auto angb = std::atan2(b.x, b.y);
                      if (angb > M_PI)        { angb -= 2 * M_PI; }
                      else if (angb <= -M_PI) { angb += 2 * M_PI; }

                      return anga < angb;
                  });
        m_updatePoints();
        m_avgPoints();
    }

    static Polygon CreateRegular(vec2f pos, float rot, size_t count, float dist);
    static Polygon CreateFromAABB(const AABB& aabb);
    static Polygon CreateFromPoints(std::vector<vec2f> verticies);

    friend void draw(sf::RenderWindow& rw, const Polygon& poly, Color clr);
    friend void drawFill(sf::RenderWindow& rw, const Polygon& poly, Color clr);
    friend void drawOutline(sf::RenderWindow& rw, const Polygon& poly, Color clr);
};


// hue: 0-360°; sat: 0.f-1.f; val: 0.f-1.f

vec2f operator* (vec2f a, vec2f b);
}
