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
struct Map {
    size_t w;
    size_t h;

    std::vector<CellVar> data;
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

    std::vector<vec2i> m_PixelsToClean;
    //'camera' window
    AABBi m_ViewWindow;

    //returned when geet is out of bounds
    CellVar null_obj = CellVar(eCellType::Bedrock);

    inline size_t m_idx(int x, int y) const { return x + y * m_width; }

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

    inline size_t getWidth() const { return m_width; }
    inline size_t getHeight() const { return m_height; }
    inline vec2u getSize() const { return vec2u(m_width, m_height); }
    inline AABBi getDefaultViewWindow() { return { vec2i(0, 0), vec2i(m_width, m_height) }; }

    inline AABBi getViewWindow() const { return m_ViewWindow; }

    void setViewWindow(AABBi aabb) {
        m_ViewWindow = aabb;

        sf::Image t(m_Buffer);
        m_Buffer.create(aabb.size().x, aabb.size().y, CellVar::properties[eCellType::Air].colors.front());

        m_updateSegment(aabb.min, aabb.max);
        m_redrawSegment(aabb);
    }

    inline bool inBounds(int x, int y) const {
        return x >= 0 && y >= 0 && x < m_width && y < m_height;
    }
    inline bool inBounds(vec2i v) const { return inBounds(v.x, v.y); }
    inline bool inView(int x, int y) { 
        return 
            x >= m_ViewWindow.min.x  && x < m_ViewWindow.max.x &&
            y >= m_ViewWindow.min.y  && y < m_ViewWindow.max.y;
    }


    inline const CellVar& get(int x, int y) const {
        if(!inBounds(x, y))
            return null_obj;
        return m_plane[m_idx(x, y)];
    }
    inline const CellVar& get(vec2i v) const {
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
    //will refresh this pixel on next update
    void drawCellAtClean(int x, int y, clr_t color);

    void updateChangedSegments();

    void redrawChangedSegments();
    void render(window_t& rw);

    //rect in which other should be fitted(fixed point will be the coordinate of bl_corner in rect)
    void mergeAt(const Grid& other, AABBi rect = AABBi(vec2i(0, 0), vec2i(-1, -1)), vec2i fixed = vec2i(0, 0));

    void exportToFile(const char* filename);
    void importFromFile(const char* filename);

    static Map importData(const char* filename);

    Grid(int w, int h, const std::vector<CellVar>& vec = std::vector<CellVar>());
    friend QuarrySprite;
};
