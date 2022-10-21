#pragma once
#include "utils.h"
#include "sprite.h"
#include "rigidbody.h"

class Entity {
    unsigned int m_id;
    static unsigned int genID() {
        static unsigned int id = 0;
        return id++;
    }
public:
    QuarrySprite spr;
    CircleRigidbody rb;
    unsigned int type;
    inline unsigned int getID() const {
        return m_id;
    }
    //in degrees

    vec2f pos;

    inline void draw(Grid& g) {
        spr.drawAt((vec2i)pos, g);
    }
    inline void processEntityCollisions(Entity* other) {
        if(rb.processRigidbodyCol(other->rb, pos, other->pos)) {
            onHitEntity();
            other->onHitEntity();
        }
    }
    virtual void onHitEntity() {}

    virtual void update(Grid& g) { }

    Entity(vec2f p, const QuarrySprite& sprite) : pos(p), spr(sprite), m_id(genID()) {
        float avg = length(vec2f(sprite.getWidth(), sprite.getHeight()));
        rb.radius = avg / 2.f;
    }
};
