#pragma once
#include <vector>
#include <iostream>
#include <functional>

#include "utils.h"
#include "cell.h"

class GridSprite;

class Grid {
private:
    size_t m_width;
    size_t m_height;

    size_t tick_passed_total = 0;

    std::vector<CellVar> m_plane;
    std::vector<CellVar> m_tmp_plane;

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
    void m_drawSegment(vec2i min, vec2i max, window_t& rw);

    void m_redrawSegment(AABBi redraw_window);
public:
    std::vector<AABBi> m_ChangedSectors;
    AABBi getViewWindow() const {
        return m_ViewWindow;
    }
    void setViewWindow(AABBi aabb) {
        m_ViewWindow = aabb;

        sf::Image t(m_Buffer);
        m_Buffer.create(aabb.size().x, aabb.size().y, CellVar::properties[eCellType::Air].colors.front());

        m_Buffer.copy(t, 0, std::clamp<int>(aabb.size().y - t.getSize().y - 1, 0, aabb.size().y));
    }

    size_t time_step = 1;
    inline bool inBounds(int x, int y) {
        return x >= 0 && y >= 0 && x < m_width && y < m_height;
    }
    inline bool inBounds(vec2i v) { return inBounds(v.x, v.y); }
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

    vec2i convert_coords(vec2i mouse_pos, window_t& window);
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

    Grid(int w, int h) : m_width(w), m_height(h) {
        m_Buffer.create(w, h, CellVar::properties[eCellType::Air].colors.front());
        for(int i = 0; i < h; i++)
            for(int ii = 0; ii < w; ii++)
                if(i == 0 || i == h - 1 || ii == 0 || ii == w - 1)
                    m_plane.push_back(CellVar(eCellType::Bedrock));
                else
                    m_plane.push_back(CellVar(eCellType::Air));;
        setViewWindow({{0, 0}, {w, h}});
    }
    friend GridSprite;
};
