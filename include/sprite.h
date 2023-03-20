#pragma once
#include "SFML/Graphics/RenderTarget.hpp"
#include "SFML/Graphics/Sprite.hpp"
#include "SFML/Graphics/Text.hpp"
#include "SFML/Graphics/Texture.hpp"
#include "utils.h"
#include <iostream>

class Grid;

class QuarrySprite {

    unsigned int m_width, m_height;
    vec2i last_pos = {0xffffff, 0xffffff};
    sf::Sprite m_spr;
    sf::Texture m_tex;
public:
    struct {
        bool x = false;
        bool y = false;
    }flip;
    inline int getWidth() const                     { return m_width; }
    inline int getHeight() const                    { return m_height; }

    void drawAt(vec2i pos, sf::RenderTarget& rw, const Grid& grid) const;

    QuarrySprite(unsigned int w, unsigned int h) : m_width(w), m_height(h) { }

    QuarrySprite(const char* filename);
    friend Grid;
};
