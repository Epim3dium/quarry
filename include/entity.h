#pragma once
#include "utils.h"
#include "grid.h"

enum class eEntityType {
    None,
    Humanoid,
};

class Entity {
    unsigned int m_getNextId() {
        static unsigned int s_next_id = 1;
        return s_next_id++;
    }
    unsigned int id;
public:
    inline unsigned int getID() const { return id; }

    eEntityType type;
    vec2f pos;
    virtual void update(Grid& grid) {}
    virtual void draw(Grid& grid, window_t& rw) {}
    Entity(eEntityType type_ = eEntityType::None) : id(m_getNextId()), type(type_) {}
};
