#pragma once
#include <vector>
#include <iostream>
#include <functional>

#include "utils.h"
#include "cell.h"

class QuarrySprite;

struct SegInfo {
    bool toUpdate = false;
    bool toUpdateNextFrame = false;
};
class Grid {
private:
    size_t m_width;
    size_t m_height;

    size_t tick_passed_total = 0;

    std::vector<CellVar> m_plane;

    std::vector<SegInfo> m_section_list;

    std::vector<AABBi> m_SegmentsToRerender;
    sf::Image m_Buffer;
    AABBi m_ViewWindow;

    //returned when geet is out of bounds
    CellVar null_obj = CellVar(eCellType::Bedrock);

    inline size_t m_idx(int x, int y) {
        return x + y * m_width;
    }
    void updateCell(int x, int y);

    void m_updateSegment(vec2i min, vec2i max);
    void m_redrawSegment(AABBi redraw_window);
    void m_update(AABBi redraw_window);

    void m_analyzeRow(int id_y);


    std::vector<AABBi> m_debugAABBDraw;
    std::vector<AABBi> m_debugAABBUpdate;
    void m_drawDebug(window_t& window);
public:
    struct {
        bool isActive = false;
        bool showUpdated = true;
        bool showDraws = true;
        clr_t update_clr = clr_t::Red;
        clr_t draw_clr = clr_t::Green;
    }debug;
    AABBi getViewWindow() const {
        return m_ViewWindow;
    }
    void setViewWindow(AABBi aabb) {
        m_ViewWindow = aabb;

        sf::Image t(m_Buffer);
        m_Buffer.create(aabb.size().x, aabb.size().y, CellVar::properties[eCellType::Air].colors.front());

        m_redrawSegment(aabb);
    }

    size_t time_step = 1;
    inline bool inBounds(int x, int y) {
        return x >= 0 && y >= 0 && x < m_width && y < m_height;
    }
    inline bool inBounds(vec2i v) { return inBounds(v.x, v.y); }
    inline bool inView(int x, int y) { 
        return 
            x >= m_ViewWindow.min.x  && x < m_ViewWindow.max.x &&
            y >= m_ViewWindow.min.y  && y < m_ViewWindow.max.y;
    }
    inline AABBi getDefaultViewWindow() {
        return { vec2i(0, 0), vec2i(m_width, m_height) };
    }


    inline const CellVar& get(int x, int y) {
        if(!inBounds(x, y))
            return null_obj;
        return m_plane[m_idx(x, y)];
    }
    inline const CellVar& get(vec2i v) {
        return get(v.x, v.y);
    }
    void set(int x, int y, const CellVar& cv);
    inline void set(vec2i v, const CellVar& cv) {
        return set(v.x, v.y, cv);
    }

    inline void swap_at(vec2i v0, vec2i v1) {
        auto t = get(v0);
        t.move_count++;
        auto tt = get(v1); 
        tt.move_count++;
        set(v0, tt);
        set(v1, t);
    }

    vec2i convert_coords(vec2i mouse_pos, vec2f win_size);
    void drawCellAt(int x, int y, clr_t color);

    void updateChangedSegments();

    void redrawChangedSegments();
    void render(window_t& rw);

    inline void update() {
        auto t = getDefaultViewWindow();
        for(int i = 0; i < time_step; i++){
            tick_passed_total++;
            m_updateSegment(t.min, t.max);
        }
    }

    Grid(int w, int h);
    friend QuarrySprite;
};
