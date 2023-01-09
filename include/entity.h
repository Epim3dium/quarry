#pragma once
#include <variant>
#include <optional>
#include <map>

#include "utils.h"
#include "sprite.h"
#include "rigidbody.h"

class Entity {
    unsigned int m_id;
    static unsigned int genID() {
        static unsigned int id = 0;
        return id++;
    }
    std::map<std::string, std::variant<vec2f, float, std::string, void*> > var;
public:
    void SetVar(std::string id, float val) {
        var[id] = val;
    }
    void SetVar(std::string id, vec2f val) {
        var[id] = val;
    }
    void SetVar(std::string id, std::string val) {
        var[id] = val;
    }
    void SetVar(std::string id, void* val) {
        var[id] = val;
    }
    template<class T>
    std::optional<T> GetVar(std::string id) {
        if(var.find(id) != var.end()) {
            return std::get<T>(var[id]); 
        }
        return {};
    }

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
            onHitEntity(other);
            other->onHitEntity(this);
        }
    }
    virtual void onHitEntity(Entity* e) {}

    virtual void update(Grid& g) { }

    Entity(vec2f p, const QuarrySprite& sprite) : pos(p), spr(sprite), m_id(genID()) {
        float avg = length(vec2f(sprite.getWidth(), sprite.getHeight()));
        rb.radius = avg / 2.f;
    }
};
