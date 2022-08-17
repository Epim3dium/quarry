#include "grid.h"
#include "timer.h"
#include "grid_sprite.h"

void Grid::set(int x, int y, const CellVar& cv) {
        AABBi* cur_sector = nullptr;
        for(auto& seg : m_ChangedSectors) {
            if(seg.contains({x, y})) {
                cur_sector = &seg;
                break;
            }
            if(seg.distance(x, y) > MAX_CHANGED_SEG_DIST)
                continue;
            cur_sector = &seg;
            cur_sector->min.x = std::min(x, cur_sector->min.x);
            cur_sector->min.y = std::min(y, cur_sector->min.y);

            cur_sector->max.x = std::max(x, cur_sector->max.x);
            cur_sector->max.y = std::max(y, cur_sector->max.y);
            break;
        }
        if(cur_sector == nullptr) {
            m_ChangedSectors.push_back({ {0xffffff, 0xffffff}, {0, 0} });
            cur_sector = &m_ChangedSectors.back();
            cur_sector->min.x = std::min(x, cur_sector->min.x);
            cur_sector->min.y = std::min(y, cur_sector->min.y);

            cur_sector->max.x = std::max(x, cur_sector->max.x);
            cur_sector->max.y = std::max(y, cur_sector->max.y);
        }
            

        if(!inBounds(x, y))
            exit(1);
        m_plane[m_idx(x, y)] = cv;
    }
void Grid::m_updateSegment(vec2i min, vec2i max) {
    //high cap has to be 1 less so that max will not be equal min
    min.x = std::clamp<int>(min.x, 0, m_width - 1);
    min.y = std::clamp<int>(min.y, 0, m_height - 1);

    max.x = std::clamp<int>(max.x, min.x + 1, m_width);
    max.y = std::clamp<int>(max.y, min.y + 1, m_height);

    bool left_to_right = tick_passed_total % 4;

    m_tmp_plane = m_plane;
    if(left_to_right)
        for(int y = min.y; y < max.y; y++) {
            for(int x = min.x; x < max.x; x++) {
                updateCell(x, y);
            }
        }
    else 
        for(int y = min.y; y < max.y; y++) {
            for(int x = max.x - 1; x >= min.x; x--) {
                updateCell(x, y);
            }
        }
}
#define CHANGED_SEGMENT_PADDING 2
void Grid::updateChangedSegments() {
    std::vector<AABBi> tmp = m_ChangedSectors;
    m_ChangedSectors.clear();

    for(int i = 0; i < time_step; i++) {
        tick_passed_total ++;
        for(auto& seg : tmp) {
            auto t = seg;
            m_SegmentsToRerender.push_back(t);
            seg = { {(int)0xffffff, (int)0xffffff}, {(int)0, (int)0} };

            vec2i padding = {CHANGED_SEGMENT_PADDING, CHANGED_SEGMENT_PADDING};
            m_updateSegment(t.min - padding, t.max + padding);
        }
    }
};
void Grid::redrawChangedSegments(window_t& rw, AABBi view_window) {
    for(auto& seg : m_SegmentsToRerender) {
        vec2i padding = {CHANGED_SEGMENT_PADDING, CHANGED_SEGMENT_PADDING};
        if(seg.min.x != (int)0xffffff)
            m_redrawSegment({ seg.min - padding, seg.max + padding}, view_window, rw);
    }
    m_SegmentsToRerender.clear();
}

void Grid::updateCell(int x, int y) {
    if(get(x, y).type == eCellType::Air)
        return;
    if(get(x, y).getID() != m_tmp_plane[m_idx(x, y)].getID())
        return;
    if(get(x, y).last_tick_updated >= tick_passed_total) 
        return;
    auto& cur_prop = CellVar::properties[get(x, y).type];
    if(x > 0 && y > 0 && x < m_width - 1 && y < m_height - 1)
        do {
            if(check_reactions({x, y}, {x + 1, y}))
                continue;
            if(check_reactions({x, y}, {x - 1, y}))
                continue;
            if(check_reactions({x, y}, {x, y + 1}))
                continue;
            if(check_reactions({x, y}, {x, y - 1}))
                continue;
        }while(false);
    //using m_idx to bypass setting and causing unnecessary checks
    m_plane[m_idx(x, y)].last_tick_updated = tick_passed_total;
    m_plane[m_idx(x, y)].age++;
    cur_prop.update_behaviour({x, y}, *this);
}
//redraw window defines what segment should be redrawn and view_window the size of full window that should fit on the screen
void Grid::m_redrawSegment(AABBi redraw_window, AABBi view_window, window_t& rw) {
    redraw_window.min.x = std::clamp<int>(redraw_window.min.x, 0, m_width);
    redraw_window.min.y = std::clamp<int>(redraw_window.min.y, 0, m_height);

    redraw_window.max.x = std::clamp<int>(redraw_window.max.x, 0, m_width);
    redraw_window.max.y = std::clamp<int>(redraw_window.max.y, 0, m_height);
    //^^clamping everything
    vec2i grid_size = view_window.size();

    vec2u size = rw.getSize();
    vec2f seg_size;
    seg_size.x = (float)size.x / (float)(grid_size.x);
    seg_size.y = (float)size.y / (float)(grid_size.y);

    for(int y = redraw_window.min.y; y < redraw_window.max.y; y++) {
        for(int x = redraw_window.min.x; x < redraw_window.max.x; x++) {
            sf::RectangleShape t(seg_size);
            t.setFillColor(get(x, y).color);
            t.setPosition({(x - view_window.min.x) * seg_size.x, (float)size.y - (y - view_window.min.y) * seg_size.y});
            rw.draw(t);
        }
    }
}
void Grid::drawCellAt(int x, int y, AABBi view_window, clr_t color, window_t& rw) {
    x = std::clamp<int>(x, 0, m_width);
    y = std::clamp<int>(y, 0, m_height);
    vec2i grid_size = view_window.size();

    vec2u size = rw.getSize();
    vec2f seg_size;
    seg_size.x = (float)size.x / (float)(grid_size.x);
    seg_size.y = (float)size.y / (float)(grid_size.y);

    sf::RectangleShape t(seg_size);
    t.setFillColor(color);
    t.setPosition({(x - view_window.min.x) * seg_size.x, (float)size.y - (y - view_window.min.y) * seg_size.y});
    rw.draw(t);
}
void Grid::drawSpriteAt(const GridSprite& sprite, vec2i pos, const AABBi& view_window, window_t& rw) {
    vec2i grid_size = view_window.size();

    vec2u size = rw.getSize();
    vec2f seg_size;
    seg_size.x = (float)size.x / (float)(grid_size.x);
    seg_size.y = (float)size.y / (float)(grid_size.y);

    int w = sprite.getWidth();
    int h = sprite.getHeight();

    pos -= {w/2, h/2};

    for(int y = 0; y < h; y++) {
        for(int x = 0; x < w; x++) {
            sf::RectangleShape t(seg_size);
            t.setFillColor(sprite.get(x, y));
            t.setPosition({(pos.x + x - view_window.min.x) * seg_size.x, (float)size.y - (pos.y + y - view_window.min.y) * seg_size.y});
            rw.draw(t);
        }
    }
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
bool Grid::check_reactions(vec2i v0, vec2i v1) {
    bool hasReacted = false;
    eCellType t0(get(v0).type);
    auto& t0_properties = CellVar::properties.at(t0);
    eCellType t1(get(v1).type);
    auto& t1_properties = CellVar::properties.at(t1);

    float reaction_chance = CellVar::rng.Random();
    //if t0 has a reaction with t1
    if(t0_properties.reactions.contains(t1)) {
        auto& reaction_info = t0_properties.reactions.at(t1);

        bool reaction_sustained = reaction_chance < reaction_info.probability;
        if(reaction_sustained) {
            set(v0, CellVar(t0_properties.reactions.at(t1).product));
            hasReacted = true;
        }
    }
    if(t1_properties.reactions.contains(t0)) {
        auto& reaction_info = t1_properties.reactions.at(t0);

        bool reaction_sustained = reaction_chance < reaction_info.probability;
        if(reaction_sustained) {
            set(v1, CellVar(reaction_info.product));
            hasReacted = true;
        }
    }
    return hasReacted;
}
vec2i Grid::convert_coords(vec2i px_pos, window_t& window) {
    vec2f size = window.getDefaultView().getSize();
    vec2f seg_size;
    seg_size.x = size.x / m_width;
    seg_size.y = size.y / m_height;
    int pos_x = int(px_pos.x /  seg_size.x); 
    int pos_y = int(size.y/seg_size.y - px_pos.y / seg_size.y);
    pos_x = std::clamp<int>(pos_x, 0, m_width - 1);
    pos_y = std::clamp<int>(pos_y, 0, m_height - 1);
    return {pos_x, pos_y};
}
