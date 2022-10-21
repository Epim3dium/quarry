#pragma once
#include "utils.h"
#include "lodepng.h"
#include <iostream>

class Grid;

class QuarrySprite {

    unsigned int m_width, m_height;
    std::vector<clr_t> buffer;
    vec2i last_pos = {0xffffff, 0xffffff};

    void m_drawBufAt(std::vector<clr_t>& buf, vec2i pos, Grid& grid);
public:
    struct {
        bool x = false;
        bool y = false;
    }flip;
    inline clr_t* getPixels() {
        if(buffer.size() != 0)
            return &buffer[0];
        return nullptr;
    }
    inline int getWidth() const                     { return m_width; }
    inline int getHeight() const                    { return m_height; }
    
    inline clr_t get(int x, int y) const            { return buffer[x + y * m_width];     }
    inline clr_t get(vec2i v) const                 { return get(v.x, v.y);   }

    inline void set(int x, int y, const clr_t& clr) { buffer[x + y * m_width] = clr;  }
    inline void set(vec2i v, const clr_t& clr)      { set(v.x, v.y, clr);     }

    void drawAt(vec2i pos, Grid& grid);

    QuarrySprite(unsigned int w, unsigned int h, clr_t clr = clr_t::Transparent) : m_width(w), m_height(h), buffer(w*h, clr) {  }
    QuarrySprite(unsigned int w, unsigned int h, std::vector<clr_t> clrs) : m_width(w), m_height(h), buffer(clrs) { }

    QuarrySprite(const char* filename);
    friend Grid;
};
