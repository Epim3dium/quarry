#include "grid.h"
#include "timer.h"
#include "grid_sprite.h"

#define MAX_CHANGED_SEG_DIST 64.f
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
    epi::timer::scope timer("update");
    //high cap has to be 1 less so that max will not be equal min
    min.x = std::clamp<int>(min.x, 0, m_width - 1);
    min.y = std::clamp<int>(min.y, 0, m_height - 1);

    max.x = std::clamp<int>(max.x, min.x + 1, m_width);
    max.y = std::clamp<int>(max.y, min.y + 1, m_height);

    bool left_to_right = tick_passed_total % 2;

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
#define MAX_MOVE_COUNT 1024U
void Grid::updateCell(int x, int y) {
    if(get(x, y).type == eCellType::Air)
        return;
    if(get(x, y).getID() != m_tmp_plane[m_idx(x, y)].getID())
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
            t.min -= padding;
            t.max += padding;
            if(AABBvAABB(t, m_ViewWindow))
                m_updateSegment(t.min, t.max);
        }
    }
};
void Grid::redrawChangedSegments() {
    for(auto& seg : m_SegmentsToRerender) {
        vec2i padding = {CHANGED_SEGMENT_PADDING, CHANGED_SEGMENT_PADDING};
        if(seg.min.x != (int)0xffffff )
            m_redrawSegment({ seg.min - padding, seg.max + padding});
    }
    m_SegmentsToRerender.clear();
}

//redraw window defines what segment should be redrawn and view_window the size of full window that should fit on the screen
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
vec2i Grid::convert_coords(vec2i mouse_pos, window_t& window) {
    vec2f size = window.getDefaultView().getSize();
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
