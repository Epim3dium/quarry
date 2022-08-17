#pragma once
#include "utils.h"
#include "lodepng.h"
#include <iostream>

class GridSprite {

    unsigned int m_width, m_height;
    std::vector<clr_t> buffer;
public:
    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }
    
    clr_t* getPixels() {
        if(buffer.size() != 0)
            return &buffer[0];
        return nullptr;
    }
    clr_t get(int x, int y) const { 
        return buffer[x + y * m_width]; 
    }
    clr_t get(vec2i v) const {
        return get(v.x, v.y);
    }
    void set(int x, int y, const clr_t& clr) { 
        buffer[x + y * m_width] = clr; 
    }
    void set(vec2i v, const clr_t& clr) {
        set(v.x, v.y, clr);
    }

    GridSprite(unsigned int w, unsigned int h, clr_t clr = clr_t::Transparent) : m_width(w), m_height(h), buffer(w*h, clr) {}
    GridSprite(unsigned int w, unsigned int h, std::vector<clr_t> clrs) : m_width(w), m_height(h), buffer(clrs) {}
    GridSprite(const char* filename) {

        std::vector<unsigned char> png;

        //load and decode
        unsigned error = lodepng::load_file(png, filename);

        static_assert(sizeof(unsigned char) == sizeof(sf::Uint8));

        if(!error) error = lodepng::decode(*(std::vector<unsigned char>*)&buffer, m_width, m_height, png);
        std::reverse(buffer.begin(), buffer.end());

        //if there's an error, display it
        if(error) std::cout << "decoder error " << error << ": " << lodepng_error_text(error) << std::endl;
    }
};
