#include "sprite.h"
#include "SFML/Graphics/RenderTarget.hpp"
#include "SFML/Graphics/RenderWindow.hpp"
#include "SFML/Graphics/Texture.hpp"
#include "grid.hpp"
#include "utils.h"

void QuarrySprite::drawAt(vec2i pos, sf::RenderTarget& rw, const Grid& grid) const {
    auto vw = grid.getViewWindow();
    auto tmpspr = m_spr;
    tmpspr.setPosition((vec2f)pos);
    rw.draw(tmpspr);
    
}

QuarrySprite::QuarrySprite(const char* filename) {
    std::vector<unsigned char> png;
    m_tex.loadFromFile(filename);
    m_spr.setTexture(m_tex);

    //load and decode

    //if there's an error, display it
}
