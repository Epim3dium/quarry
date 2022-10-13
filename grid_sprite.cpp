#include "grid_sprite.h"
#include "grid.h"
#include "utils.h"

void GridSprite::m_drawBufAt(std::vector<clr_t>& buf, vec2i pos, Grid& grid) {
    vec2i grid_size = grid.m_ViewWindow.size();

    int w = getWidth();
    int h = getHeight();

    pos -= {w/2, h/2};

    for(int y = 0; y < h; y++) {
        for(int x = 0; x < w; x++) {
            if(buf[x + y * w].a != 0.f && VecvAABB(vec2i(pos.x + x, pos.y + y), grid.getViewWindow()))
                grid.drawCellAt(pos.x + x, pos.y + y, buf[x + y * w]);
        }
    }
}
void GridSprite::drawAt(vec2i pos, Grid& grid) {
    auto size = vec2i(m_width, m_height);
    if(last_pos.x != 0xffffff && last_pos.y != 0xffffff) {
        for(int y = 0; y < m_width; y++) {
            for(int x = 0; x < m_height; x++) {
                if(grid.inBounds(vec2i(last_pos) + vec2i(x, y) - size / 2 ))
                    background[x + y * m_width] = grid.get(vec2i(last_pos) + vec2i(x, y)).color;
            }
        }
    }
    //drawing pixel buffer
    if(last_pos.x != 0xffffff && last_pos.y != 0xffffff) {
        vec2i size(getWidth() + 1, getHeight() + 1);
        grid.m_redrawSegment(AABBi{last_pos - size / 2 , last_pos + size / 2});
    }
    m_drawBufAt(buffer, pos, grid );
    last_pos = pos;
}

GridSprite::GridSprite(const char* filename) {
    std::vector<unsigned char> png;

    //load and decode
    unsigned error = lodepng::load_file(png, filename);

    static_assert(sizeof(unsigned char) == sizeof(sf::Uint8));

    if(!error) error = lodepng::decode(*(std::vector<unsigned char>*)&buffer, m_width, m_height, png);
    std::reverse(buffer.begin(), buffer.end());

    //if there's an error, display it
    if(error) std::cout << "decoder error " << error << ": " << lodepng_error_text(error) << std::endl;
    background = buffer;
}
