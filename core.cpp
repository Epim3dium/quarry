#include "core.h"
#include "timer.h"
#include "sprite.h"
#include "opt.h"

Grid::Grid(int w, int h) : m_width(w), m_height(h), m_section_list(w / UPDATE_SEG_SIZE * h / UPDATE_SEG_SIZE){
    if(CellVar::properties.size() != (int)eCellType::Bedrock + 1) {
        std::cerr << "Properties uninitialized";
        std::exit(0);
    }
    m_Buffer.create(w, h, CellVar::properties[eCellType::Air].colors.front());
    for(int i = 0; i < h; i++)
        for(int ii = 0; ii < w; ii++)
            if(i == 0 || i == h - 1 || ii == 0 || ii == w - 1)
                m_plane.push_back(CellVar(eCellType::Bedrock));
            else
                m_plane.push_back(CellVar(eCellType::Air));;
    setViewWindow({{0, 0}, {w, h}});
}
void Grid::set(int x, int y, const CellVar& cv) {
    int id_x = x / UPDATE_SEG_SIZE;
    int id_y = y / UPDATE_SEG_SIZE;
    m_section_list[id_x + id_y * m_width / UPDATE_SEG_SIZE].toUpdateNextFrame = true;
    if(x % UPDATE_SEG_SIZE == 0 && id_x != 0) {
        m_section_list[id_x - 1 + id_y * m_width / UPDATE_SEG_SIZE].toUpdateNextFrame = true;
    }
    if(x % UPDATE_SEG_SIZE == UPDATE_SEG_SIZE - 1 && id_x != m_width / UPDATE_SEG_SIZE - 1) {
        m_section_list[id_x + 1 + id_y * m_width / UPDATE_SEG_SIZE].toUpdateNextFrame = true;
    }
    if(y % UPDATE_SEG_SIZE == 0 && id_y != 0) {
        m_section_list[id_x + (id_y - 1) * m_width / UPDATE_SEG_SIZE].toUpdateNextFrame = true;
    }
    if(y % UPDATE_SEG_SIZE == UPDATE_SEG_SIZE - 1 && id_y != m_height / UPDATE_SEG_SIZE - 1) {
        m_section_list[id_x + (id_y + 1) * m_width / UPDATE_SEG_SIZE].toUpdateNextFrame = true;
    }


    if(!inBounds(x, y))
        exit(1);
    m_plane[m_idx(x, y)] = cv;
}
void Grid::m_updateSegment(vec2i min, vec2i max) {
    epi::timer::scope timer("update");
    //high cap has to be 1 less so that max will not be equal min
    min.x = std::clamp<int>(min.x, 0, m_width - 1);
    min.y = std::clamp<int>(min.y, 0, m_height - 1);

    max.x = std::clamp<int>(max.x, min.x + 1, m_width);
    max.y = std::clamp<int>(max.y, min.y + 1, m_height);

    //other sides to prevent weird lags
    bool left_to_right = tick_passed_total % 2;
#define DOWN_UP\
        for(int y = min.y; y < max.y; y++)
#define LEFT_RIGHT\
        for(int x = min.x; x < max.x; x++)
#define RIGHT_LEFT\
        for(int x = max.x - 1; x >= min.x; x--)
    if(left_to_right)
       DOWN_UP 
            LEFT_RIGHT
                updateCell(x, y);
    else 
       DOWN_UP 
           RIGHT_LEFT 
                updateCell(x, y);

}
void Grid::updateCell(int x, int y) {
    if(get(x, y).type == eCellType::Air)
        return;
    if(get(x, y).last_tick_updated >= tick_passed_total) 
        return;
    if(get(x, y).move_count > MAX_MOVE_COUNT) {
        set(x, y, eCellType::Air);
        return;
    }
    auto& cur_prop = CellVar::properties[get(x, y).type];
    //using m_idx to bypass setting and causing unnecessary checks
    m_plane[m_idx(x, y)].last_tick_updated = tick_passed_total;
    m_plane[m_idx(x, y)].age++;
    cur_prop.update_behaviour({x, y}, *this);
}
void Grid::m_updateSection(size_t index) {
    bool shouldUpdate = m_section_list[index].toUpdate;
    m_section_list[index].toUpdate = false;
    if(m_section_list[index].toUpdateNextFrame) {
        m_section_list[index].toUpdate = true;
        m_section_list[index].toUpdateNextFrame = false;
    }
    if(!shouldUpdate)
        return;

    int bl_x = (index * UPDATE_SEG_SIZE) % m_width;
    int bl_y = (index * UPDATE_SEG_SIZE) / m_width * UPDATE_SEG_SIZE;
    AABBi seg = {vec2i(bl_x, bl_y), vec2i(bl_x + UPDATE_SEG_SIZE, bl_y + UPDATE_SEG_SIZE)};
    m_SegmentsToRerender.push_back(seg);

    if(AABBvAABB(seg, m_ViewWindow))
        m_updateSegment(seg.min, seg.max);
}
void Grid::updateChangedSegments() {
    tick_passed_total ++;
    bool side_x = g_rng.Random() > 0.5f;
    bool side_y = g_rng.Random() > 0.5f;

    //if((i % (m_width / UPDATE_SEG_SIZE)) % 2 == 0)
    int ctr = 0;
    int id_y = 0;
    if(side_y)
        id_y = m_height / UPDATE_SEG_SIZE - 1;
    while(id_y >= 0 && id_y < m_height / UPDATE_SEG_SIZE) {
        int id_x = 0;
        if(side_x)
            id_x = m_width / UPDATE_SEG_SIZE - 1;
        while(id_x >= 0 && id_x < m_width / UPDATE_SEG_SIZE) {
            //
                m_updateSection(id_x + id_y * m_width / UPDATE_SEG_SIZE);
            //
            if(side_x) {
                id_x--;
            } else {
                id_x++;
            }
        }
        //
        if(side_y) {
            id_y--;
        } else {
            id_y++;
        }
    }
};
void Grid::redrawChangedSegments() {
    for(auto& seg : m_SegmentsToRerender) {
        if(seg.min.x != (int)0xffffff )
            m_redrawSegment(seg);
    }
    if(toggleDebug)
        m_debugDraws = m_SegmentsToRerender;
    m_SegmentsToRerender.clear();
}

//redraw window defines what segment should be redrawn and view_window the size of full window that should fit on the screen
void Grid::m_drawDebug(window_t& rw) {
    vec2f ratio = vec2f((float)rw.getSize().x / m_ViewWindow.size().x, (float)rw.getSize().y / m_ViewWindow.size().y) ;
    for(auto& seg : m_debugDraws) {
        sf::Vertex line[2];
        line[0] = sf::Vertex(vec2f(seg.min.x * ratio.x, rw.getSize().y - seg.min.y * ratio.y));
        line[1] = sf::Vertex(vec2f(seg.max.x * ratio.x, rw.getSize().y - seg.min.y * ratio.y));

        line[0].color = clr_t::Green;
        line[1].color = clr_t::Green;
        rw.draw(line, 2, sf::Lines);

        line[0].position = vec2f(seg.min.x * ratio.x, rw.getSize().y - seg.max.y * ratio.y);
        line[1].position = vec2f(seg.max.x * ratio.x, rw.getSize().y - seg.max.y * ratio.y);
        rw.draw(line, 2, sf::Lines);
        line[0].position = vec2f(seg.min.x * ratio.x, rw.getSize().y - seg.min.y * ratio.y);
        line[1].position = vec2f(seg.min.x * ratio.x, rw.getSize().y - seg.max.y * ratio.y);
        rw.draw(line, 2, sf::Lines);
        line[0].position = vec2f(seg.max.x * ratio.x, rw.getSize().y - seg.min.y * ratio.y);
        line[1].position = vec2f(seg.max.x * ratio.x, rw.getSize().y - seg.max.y * ratio.y);
        rw.draw(line, 2, sf::Lines);

    }
    m_debugDraws.clear();
}
void Grid::m_redrawSegment(AABBi redraw_window) {

    redraw_window.min.x = std::clamp<int>(redraw_window.min.x, 0, m_width - 1);
    redraw_window.min.y = std::clamp<int>(redraw_window.min.y, 0, m_height - 1);

    redraw_window.max.x = std::clamp<int>(redraw_window.max.x, redraw_window.min.x + 1, m_width);
    redraw_window.max.y = std::clamp<int>(redraw_window.max.y, redraw_window.min.y + 1, m_height);
    //^^clamping everything
    vec2i grid_size = m_ViewWindow.size();

    for(int y = redraw_window.min.y; y < redraw_window.max.y; y++) {
        for(int x = redraw_window.min.x; x < redraw_window.max.x; x++) {
            //get(x, y).color);
            //t.setPosition({(x - view_window.min.x), (y - view_window.min.y)});
            {
                epi::timer::scope timer("draw_inner");
                if(inBounds(x - m_ViewWindow.min.x, m_ViewWindow.max.y - y - m_ViewWindow.min.y - 1))
                    m_Buffer.setPixel(x - m_ViewWindow.min.x, m_ViewWindow.max.y - y - m_ViewWindow.min.y - 1, get(x, y).color);
            }
        }
    }
}
void Grid::render(window_t& rw) {
    sf::Texture tex;
    tex.loadFromImage(m_Buffer);
    sf::Sprite spr(tex);
    vec2f ratio = vec2f((float)rw.getSize().x / tex.getSize().x, (float)rw.getSize().y / tex.getSize().y) ;
    spr.setPosition(0, 0);
    spr.setScale(ratio);
    rw.draw(spr);

    if(toggleDebug)
        m_drawDebug(rw);
}
void Grid::drawCellAt(int x, int y, clr_t color) {
    x = std::clamp<int>(x, 0, m_width);
    y = std::clamp<int>(y, 0, m_height);
    vec2i grid_size = m_ViewWindow.size();

    m_Buffer.setPixel(x - m_ViewWindow.min.x, (m_ViewWindow.max.y - y) - m_ViewWindow.min.y, color);
}
void Grid::m_drawSegment(vec2i min, vec2i max, window_t& rw) {
    min.x = std::clamp<int>(min.x, 0, m_width);
    min.y = std::clamp<int>(min.y, 0, m_height);

    max.x = std::clamp<int>(max.x, 0, m_width);
    max.y = std::clamp<int>(max.y, 0, m_height);

    vec2u size = rw.getSize();
    vec2f seg_size;
    seg_size.x = (float)size.x / (float)(max.x - min.x + 1);
    seg_size.y = (float)size.y / (float)(max.y - min.y + 1);

    for(int y = min.y; y < max.y; y++) {
        for(int x = min.x; x < max.x; x++) {
            sf::RectangleShape t(seg_size);
            t.setFillColor(get(x, y).color);
            t.setPosition({(x - min.x) * seg_size.x, (float)size.y - (y - min.y + 1) * seg_size.y});
            rw.draw(t);
        }
    }
}
vec2i Grid::convert_coords(vec2i mouse_pos, vec2f window_size) {
    vec2f size = window_size;
    vec2f seg_size;
    seg_size.x = size.x / m_ViewWindow.size().x;
    seg_size.y = size.y / m_ViewWindow.size().y;
    int pos_x = int(mouse_pos.x /  seg_size.x);
    int pos_y = int(size.y/seg_size.y - mouse_pos.y / seg_size.y);

    pos_x = std::clamp<int>(pos_x, 0, m_ViewWindow.size().x - 1);
    pos_y = std::clamp<int>(pos_y, 0, m_ViewWindow.size().y- 1);

    pos_x += m_ViewWindow.min.x;
    pos_y += m_ViewWindow.min.y;
    return {pos_x, pos_y};
}
