#include "sprite.h"
#include "core.h"
#include "utils.h"

void QuarrySprite::m_drawBufAt(std::vector<clr_t>& buf, vec2i pos, Grid& grid) {
    vec2i grid_size = grid.m_ViewWindow.size();

    int w = getWidth();
    int h = getHeight();

    pos -= {w/2, h/2};

    int gy = 0;
    int y = 0;
    if(flip.y)
        y = h - 1;

    while(y >= 0 && y < h) {
        int x = 0;
        int gx = 0;
        if(!flip.x)
            x = w - 1;
        while(x >= 0 && x < w) {
            if(buf[x + y * w].a != 0.f && VecvAABB(vec2i(pos.x + gx, pos.y + gy), grid.getViewWindow()))
                grid.drawCellAt(pos.x + gx, pos.y + gy, buf[x + y * w]);
            x++;
            gx++;
            if(!flip.x)
                x -= 2;
        }
        y++;
        gy++;
        if(flip.y)
            y -= 2;
    }
}
void QuarrySprite::drawAt(vec2i pos, Grid& grid) {
    auto size = vec2i(m_width, m_height);
    //drawing pixel buffer
    if(last_pos.x != 0xffffff && last_pos.y != 0xffffff) {
        vec2i hsize(roundf((getWidth() + 2) / 2.f), roundf((getHeight() + 2) / 2.f));
        grid.m_redrawSegment(AABBi{last_pos - hsize , last_pos + hsize});
    }
    m_drawBufAt(buffer, pos, grid );
    last_pos = pos;
}

QuarrySprite::QuarrySprite(const char* filename) {
    std::vector<unsigned char> png;

    //load and decode
    unsigned error = lodepng::load_file(png, filename);

    static_assert(sizeof(unsigned char) == sizeof(sf::Uint8));

    if(!error) error = lodepng::decode(*(std::vector<unsigned char>*)&buffer, m_width, m_height, png);
    std::reverse(buffer.begin(), buffer.end());

    //if there's an error, display it
    if(error) std::cout << "decoder error " << error << ": " << lodepng_error_text(error) << std::endl;
}
