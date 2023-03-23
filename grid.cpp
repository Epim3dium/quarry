#include <thread>
#include <fstream>

#include "SFML/Graphics/RenderTarget.hpp"
#include "SFML/Graphics/Texture.hpp"
#include "col_utils.hpp"
#include "grid.hpp"
#include "opt.h"
namespace epi {

Grid::Grid(int w, int h, const std::vector<CellVar>& vec) : m_width(w), m_height(h), m_section_list(w / UPDATE_SEG_W * h / UPDATE_SEG_H), m_plane(vec){
    if(vec.size() != w * h)
        m_plane = std::vector<CellVar>(w * h, CellVar(eCellType::Air));
    m_Buffer.create(w, h, CellVar::properties[static_cast<size_t>(eCellType::Air)].colors.front());
    setViewWindow({{0, 0}, {static_cast<float>(w), static_cast<float>(h)}});
}
void Grid::set(int x, int y, const CellVar& cv) {
    int id_x = x / UPDATE_SEG_W;
    int id_y = y / UPDATE_SEG_H;
    m_section_list[id_x + id_y * m_width / UPDATE_SEG_W].wakeUp();
    if(x % UPDATE_SEG_W == 0 && id_x != 0) {
        m_section_list[id_x - 1 + id_y * m_width / UPDATE_SEG_W].wakeUp();
    }
    if(x % UPDATE_SEG_W == UPDATE_SEG_W - 1 && id_x != m_width / UPDATE_SEG_W - 1) {
        m_section_list[id_x + 1 + id_y * m_width / UPDATE_SEG_W].wakeUp();
    }
    if(y % UPDATE_SEG_H== 0 && id_y != 0) {
        m_section_list[id_x + (id_y - 1) * m_width / UPDATE_SEG_W].wakeUp();
    }
    if(y % UPDATE_SEG_H== UPDATE_SEG_H- 1 && id_y != m_height / UPDATE_SEG_H- 1) {
        m_section_list[id_x + (id_y + 1) * m_width / UPDATE_SEG_W].wakeUp();
    }


    if(!inBounds(x, y))
        exit(1);
    m_plane[m_idx(x, y)] = cv;
}
void Grid::m_updateRect(vec2f min, vec2f max) {
    //high cap has to be 1 less so that max will not be equal min
    min.x = std::clamp<int>(min.x, 0, m_width - 1);
    min.y = std::clamp<int>(min.y, 0, m_height - 1);

    max.x = std::clamp<int>(max.x, min.x + 1, m_width);
    max.y = std::clamp<int>(max.y, min.y + 1, m_height);
    if(debug.showUpdated && debug.isActive)
        m_debugAABBUpdate.push_back({(vec2f)min, (vec2f)max});

    //other sides to prevent weird lags
    bool left_to_right = tick_passed_total % 2;
    //bool up_down = (tick_passed_total / 2) % 2;
#define UP_DOWN\
        for(int y = max.y - 1; y >= min.y; y--)
#define DOWN_UP\
        for(int y = min.y; y < max.y; y++)
#define LEFT_RIGHT\
        for(int x = min.x; x < max.x; x++)
#define RIGHT_LEFT\
        for(int x = max.x - 1; x >= min.x; x--)
#define VERT_STYLE DOWN_UP

    if(left_to_right) {
        VERT_STYLE 
            LEFT_RIGHT
                m_updateCell(x, y);
    } else {
        VERT_STYLE 
           RIGHT_LEFT 
                m_updateCell(x, y);
    }
}
void Grid::m_analyzeRow(int id_y) {
    int id_x = 0;
    bool last_updated = false;
    AABB cur_aabb = {{0xffffff, 0xffffff}, {}};
    while(id_x < m_width / UPDATE_SEG_W) {
        do {
            size_t index = id_x + id_y * m_width / UPDATE_SEG_W;
            bool shouldUpdate = m_section_list[index].toUpdate;
            m_section_list[index].toUpdate = false;
            if(m_section_list[index].toUpdateNextFrame) {
                m_section_list[index].toUpdate = true;
                m_section_list[index].toUpdateNextFrame = false;
            }
            if(!shouldUpdate) {
                if(last_updated) {
                    m_updateRect(cur_aabb.min, cur_aabb.max);
                }
                last_updated = false;
                break;
            }

            int bl_x = (index * UPDATE_SEG_W) % m_width;
            int bl_y = (index * UPDATE_SEG_W) / m_width * UPDATE_SEG_H;
            AABB seg = {vec2f(bl_x, bl_y), vec2f(bl_x + UPDATE_SEG_W, bl_y + UPDATE_SEG_H)};

            auto ext_ViewWindow = m_ViewWindow;
            auto s = ext_ViewWindow.size();
            ext_ViewWindow.min -= s;
            ext_ViewWindow.max += s;

            if(isOverlappingAABBAABB(seg, ext_ViewWindow)) {
                if(last_updated)  {
                    cur_aabb.max = seg.max;
                }else  {
                    cur_aabb.min = seg.min;
                    cur_aabb.max = seg.max;
                    last_updated = true;
                }
                m_SegmentsToRerender.push_back(seg);
            }
        }while(0);
        //
        id_x++;
    }
    if(last_updated)
        m_updateRect(cur_aabb.min, cur_aabb.max);
}
void Grid::m_updateCell(int x, int y) {
    if(get(x, y).type == eCellType::Air)
        return;
    if(get(x, y).last_tick_updated >= tick_passed_total) {
        m_plane[m_idx(x, y)].last_tick_updated = tick_passed_total;
        return;
    }
    if(get(x, y).move_count > MAX_MOVE_COUNT) {
        set(x, y, eCellType::Air);
        return;
    }
    //using m_idx to bypass setting and causing unnecessary checks
    m_plane[m_idx(x, y)].last_tick_updated = tick_passed_total;
    m_plane[m_idx(x, y)].age++;
    get(x, y).getProperty().update_behaviour({x, y}, *this);
}
void Grid::update() {
    tick_passed_total ++;
#if THREADED_UPDATE
    size_t thread_c = std::thread::hardware_concurrency();
#else
    size_t thread_c = 1;
#endif
    //if((i % (m_width / UPDATE_SEG_)) % 2 == 0)
    bool side_x = tick_passed_total % 2;
    //bool side_y = (tick_passed_total / 2) % 2;
    bool side_y = 1;

    std::vector<std::thread> workers;
    auto analyzeOnlyDivisible = [&](int id, bool fhalf) {
        int id_y = 0;
        int id_x = 0;
        if(side_y)
            id_y = m_height / UPDATE_SEG_H - 1;
        while(id_y >= 0 && id_y < m_height / UPDATE_SEG_H) {
            if((id_y / 2 + 1) % thread_c == id && id_y % 2 == fhalf)
                m_analyzeRow(id_y);
            //
            if(side_y) {
                id_y--;
            } else {
                id_y++;
            }
        }
    };
    for(int half = 0; half < 2; half++) {
#if THREADED_UPDATE
        for(int i = 0; i < thread_c; i++)
            workers.push_back(std::thread(analyzeOnlyDivisible, i, half));
        for(auto& w : workers)
            if(w.joinable())
                w.join();
#else
        analyzeOnlyDivisible(0, half);
#endif
    }
};
void Grid::redraw() {
#if bREDRAW_CHANGED_ONLY 
    for(auto& seg : m_SegmentsToRerender) {
        m_redrawRect(seg);
    }
#else
    redrawSegment(getViewWindow());
#endif
    m_SegmentsToRerender.clear();
}

//redraw window defines what segment should be redrawn and view_window the size of full window that should fit on the screen
void Grid::m_drawDebug(sf::RenderTarget& rw) {
    vec2f ratio = vec2f((float)rw.getSize().x / m_ViewWindow.size().x, (float)rw.getSize().y / m_ViewWindow.size().y) ;
#define DEBUG_DRAW_AABB(vec, clr, off)\
for(auto seg : vec) {\
seg.min -= m_ViewWindow.min;\
seg.max -= m_ViewWindow.min;\
sf::Vertex line[2];\
line[0] = sf::Vertex(vec2f(seg.min.x * ratio.x, rw.getSize().y - seg.min.y * ratio.y) + off);\
line[1] = sf::Vertex(vec2f(seg.max.x * ratio.x, rw.getSize().y - seg.min.y * ratio.y) + off);\
\
line[0].color = clr;\
line[1].color = clr;\
rw.draw(line, 2, sf::Lines);\
\
line[0].position = vec2f(seg.min.x * ratio.x, rw.getSize().y - seg.max.y * ratio.y) + off;\
line[1].position = vec2f(seg.max.x * ratio.x, rw.getSize().y - seg.max.y * ratio.y) + off;\
rw.draw(line, 2, sf::Lines);\
line[0].position = vec2f(seg.min.x * ratio.x, rw.getSize().y - seg.min.y * ratio.y) + off;\
line[1].position = vec2f(seg.min.x * ratio.x, rw.getSize().y - seg.max.y * ratio.y) + off;\
rw.draw(line, 2, sf::Lines);\
line[0].position = vec2f(seg.max.x * ratio.x, rw.getSize().y - seg.min.y * ratio.y) + off;\
line[1].position = vec2f(seg.max.x * ratio.x, rw.getSize().y - seg.max.y * ratio.y) + off;\
rw.draw(line, 2, sf::Lines);\
}
    DEBUG_DRAW_AABB(m_debugAABBDraw, debug.draw_clr, vec2f(0, 0))
    DEBUG_DRAW_AABB(m_debugAABBUpdate, debug.update_clr, vec2f(2, 2))
    m_debugAABBUpdate.clear();
    m_debugAABBDraw.clear();
}
void Grid::m_redrawRect(AABB redraw_window) {
    redraw_window.min.x = std::clamp<int>(redraw_window.min.x, m_ViewWindow.min.x, m_ViewWindow.max.x - 1);
    redraw_window.min.y = std::clamp<int>(redraw_window.min.y, m_ViewWindow.min.y, m_ViewWindow.max.y - 1);

    redraw_window.max.x = std::clamp<int>(redraw_window.max.x, redraw_window.min.x + 1, m_ViewWindow.max.x);
    redraw_window.max.y = std::clamp<int>(redraw_window.max.y, redraw_window.min.y + 1, m_ViewWindow.max.y);
    if(debug.isActive && debug.showDraws)
        m_debugAABBDraw.push_back(redraw_window);
    //^^clamping everything

    for(int y = redraw_window.min.y; y < redraw_window.max.y; y++) {
        for(int x = redraw_window.min.x; x < redraw_window.max.x; x++) {
            //get(x, y).color);
            //t.setPosition({(x - view_window.min.x), (y - view_window.min.y)});
            {
                m_Buffer.setPixel(x - m_ViewWindow.min.x, m_ViewWindow.size().y - (y - m_ViewWindow.min.y) - 1, get(x, y).color);
            }
        }
    }
}
void Grid::render(sf::RenderTexture& rw, sf::Shader* frag_shader) {
    sf::Texture tex;
    tex.loadFromImage(m_Buffer, {(vec2i)m_ViewWindow.min, (vec2i)m_ViewWindow.size()});
    sf::Sprite spr(tex);
    spr.setPosition(0, 0);
    rw.draw(spr, frag_shader);

    if(debug.isActive)
        m_drawDebug(rw);
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
void Grid::mergeAt(const Grid& other, AABB rect, vec2f fixed) {
    if(rect.min.x == 0 && rect.max.x == -1) {
        rect.min.x = 0;
        rect.max.x = getWidth();

        rect.min.y = 0;
        rect.max.y = getHeight();
    }
    for(int y = rect.min.y; y < rect.max.y + other.getHeight(); y++) {
        int y_rel = y - rect.min.y + fixed.y;
        if(y_rel >= other.getHeight() || y_rel < 0)
            continue;
        for(int x = rect.min.x; x < rect.max.x + other.getWidth(); x++) {
            int x_rel = x - rect.min.x + fixed.x;
            if(x_rel >= other.getWidth() || x_rel < 0)
                continue;
            if(x >= 0 && x < getWidth() && x >= rect.min.x && x <= rect.max.x
                && y >= 0 && y < getHeight() && y >= rect.min.y && x <= rect.max.y)
                set(x, y, other.get(x - rect.min.x + fixed.x, y - rect.min.y + fixed.y));
        }
    }
    m_updateRect(rect.min, rect.max);
}
//file structure:
//width height
//      [row 0]: a b c d e ... width
//      [row 1]: a b c d e ... width 
// [row height]: a b c d e ... width
void Map::exportToFile(const char* filename) {
    std::string full_path = (std::string)IMPORT_EXPORT_MAP_DIR + filename;
    std::ofstream out(full_path, std::ios::hex | std::ios::out | std::ios::app);
    if(!out.good())
        std::cerr << "error opening a file: " << filename << "\n";
    out << w<< " " << h<< ";";
    out.write((char*)(void*)&data[0], sizeof(CellVar) * w* h);
}
void Map::importFromFile(const char* filename) {
    std::string full_path = (std::string)IMPORT_EXPORT_MAP_DIR + filename;
    std::ifstream in(full_path, std::ios::hex);
    if(!in.good())
        std::cerr << "error opening a file: " << filename << "\n";

    char end;
    in >> w >> h >> end;
    data = std::vector<CellVar>(w*h, CellVar(eCellType::Air));
    in.read((char*)(void*)&data[0], sizeof(CellVar) * w * h);
    return;
}
void Grid::importFromMap(AABB seg, const Map& m) {
    for(int y = seg.min.y; y < seg.max.y && y < m_height; y++) {
        for(int x = seg.min.x; x < seg.max.x && x < m_width; x++) {
            if(y - seg.min.y < m.h && x - seg.min.x < m.w)
                m_plane[m_idx(x, y)] = m.data[x - seg.min.x + (y - seg.min.y) * m.w];
        }
    }
}
void Grid::importFromFile(const char* filename, AABB seg) {
    Map m;
    m.importFromFile(filename);
    importFromMap(seg, m);
}
void Grid::importFromFile(const char* filename) {
    Map m;
    m.importFromFile(filename);
    *this = Grid(m.w, m.h, m.data);
}
Map Grid::exportToMap(AABB seg) {
    Map m;
    m.w = seg.size().x;
    m.h = seg.size().y;
    m.data = std::vector<CellVar>(m.w * m.h, eCellType::Air);
    for(int y = seg.min.y; y < seg.max.y; y++) {
        for(int x = seg.min.x; x < seg.max.x; x++) {
             m.data[x - seg.min.x + (y - seg.min.y) * m.w] = m_plane[m_idx(x, y)];
        }
    }
    return m;
}
void Grid::exportToFile(const char* filename, AABB seg) {
    exportToMap(seg).exportToFile(filename);
}
void Grid::exportToFile(const char* filename) {
    Map m;
    m.w = m_width;
    m.h = m_height;
    m.data = m_plane;
    m.exportToFile(filename);
}
}
