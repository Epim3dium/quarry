#pragma once
#include <vector>
#include <iostream>
#include <functional>

#include "SFML/Graphics/RenderTarget.hpp"
#include "SFML/Graphics/Texture.hpp"
#include "utils.h"
#include "cell.h"

namespace epi {
struct SegInfo {
    bool toUpdate = false;
    bool toUpdateNextFrame = false;
    void wakeUp() {
        toUpdate = true;
        toUpdateNextFrame = true;
    }
};
//basic struct for holding the barebone data
struct Map {
    size_t w;
    size_t h;

    std::vector<CellVar> data;
    void exportToFile(const char* filename);
    void importFromFile(const char* filename);
};
class Grid {
private:
    size_t m_width;
    size_t m_height;

    size_t tick_passed_total = 0;

    std::vector<CellVar> m_plane;
    std::vector<SegInfo> m_section_list;

    std::vector<AABB> m_SegmentsToRerender;
    sf::Image m_Buffer;

    //'camera' window
    AABB m_ViewWindow;

    //returned when get is out of bounds
    CellVar null_obj = CellVar(eCellType::Bedrock);

    inline size_t m_idx(int x, int y) const { return x + y * m_width; }

    void m_updateCell(int x, int y);

    void m_analyzeRow(int id_y);


    std::vector<AABB> m_debugAABBDraw;
    std::vector<AABB> m_debugAABBUpdate;
    void m_drawDebug(sf::RenderTarget& window);

    void m_updateRect(vec2f min, vec2f max);
    void m_redrawRect(AABB redraw_window);

public:
    struct {
        bool isActive = false;
        bool showUpdated = true;
        bool showDraws = true;
        Color update_clr = Color::Red;
        Color draw_clr = Color::Green;
    }debug;

    inline size_t getWidth() const { return m_width; }
    inline size_t getHeight() const { return m_height; }
    inline vec2u getSize() const { return vec2u(m_width, m_height); }
    inline AABB getDefaultViewWindow() { return { vec2f(0, 0), vec2f(m_width, m_height) }; }
    inline AABB getViewWindow() const { return m_ViewWindow; }

    void setViewWindow(AABB aabb) {
        m_ViewWindow = aabb;

        sf::Image t(m_Buffer);
        m_Buffer.create(aabb.size().x, aabb.size().y, CellVar::properties[static_cast<size_t>(eCellType::Air)].colors.front());

        m_updateRect(aabb.min, aabb.max);
        m_redrawRect(aabb);
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

    void update();
    void redraw();
    void render(sf::RenderTexture& rw, sf::Shader* frag_shader = nullptr);

    //rect in which other should be fitted(fixed point will be the coordinate of bl_corner in rect)
    void mergeAt(const Grid& other, AABB rect = AABB::CreateMinMax(vec2f(0, 0), vec2f(-1, -1)), vec2f fixed = vec2f(0, 0));


    void importFromMap(AABB seg, const Map& m);

    void importFromFile(const char* filename, AABB seg);
    void importFromFile(const char* filename);

    Map exportToMap(AABB seg);
    void exportToFile(const char* filename, AABB seg);
    void exportToFile(const char* filename);

    Grid(int w, int h, const std::vector<CellVar>& vec = std::vector<CellVar>());
};
}
