#include <iostream>

#include <SFML/Graphics.hpp>

#include "SFML/Graphics/RenderTarget.hpp"
#include "SFML/Graphics/RenderWindow.hpp"
#include "SFML/Graphics/Texture.hpp"
#include "imgui.h"
#include "imgui-SFML.h"

#include "coroutines.hpp"
#include "utils.h"
#include "grid.hpp"
#include "timer.h"
#include "sprite.h"
#include "shape.h"
#include "rigidbody.h"
#include "quarry.h"
#include "entity.h"

#define GWH 512
#define GWW 512

#define WH 2048.f
#define WW (WH * GWH/GWW)


const static QuarrySprite g_player_sprite("./assets/player.png");
const static QuarrySprite g_camera_sprite("./assets/camera.png");


class Demo : public QuarryApp {
public:

    int brush_size;
    bool on_brush_click = false;
    bool editor_mode = true;
    eCellType brush_material = eCellType::Sand;
    vec2i last_mouse_pos = {-1, -1};
    vec2i grid_mouse_coords;

    vec2f ImGuiWindowSize = vec2f(WW/5, WH / 2);

    vec2i player_input = {0, 0};

    void spawnAtBrush(Grid& grid, bool ifEmpty = false, const CellVar* cv = nullptr ) {
        vec2i mouse_pos = getMousePos();
        auto grid_mouse_coords = grid.convert_coords(mouse_pos, p_window_size); 
        for(int y = -brush_size/2; y < round((float)brush_size/2.f); y++)
            for(int x = -brush_size/2; x < round((float)brush_size/2.f); x++) {
                if(grid.inBounds(grid_mouse_coords + vec2i(x, y)) && (grid.get(grid_mouse_coords + vec2i(x, y)).type == eCellType::Air || !ifEmpty)){
                    if(cv) 
                        grid.set(grid_mouse_coords + vec2i(x, y), *cv);
                    else
                        grid.set(grid_mouse_coords + vec2i(x, y), CellVar(brush_material));
                }
            }
    }
    void spawnMaterial(Grid& grid, sf::Keyboard::Key k, eCellType type) {
        brush_material = type;
        if(sf::Keyboard::isKeyPressed(k)){
            spawnAtBrush(grid);
        }
    };
    void update(Grid& grid) override {
        if(editor_mode) {
            if (sf::Mouse::isButtonPressed(sf::Mouse::Left) && !on_brush_click) {
                spawnAtBrush(grid);
            }
            spawnMaterial(grid, sf::Keyboard::A, eCellType::Acid);
            spawnMaterial(grid, sf::Keyboard::E, eCellType::Leaf);
            spawnMaterial(grid, sf::Keyboard::O, eCellType::Wood);
            spawnMaterial(grid, sf::Keyboard::D, eCellType::Dirt);
            spawnMaterial(grid, sf::Keyboard::S, eCellType::Sand);
            spawnMaterial(grid, sf::Keyboard::W, eCellType::Water);
            spawnMaterial(grid, sf::Keyboard::L, eCellType::Lava);
            spawnMaterial(grid, sf::Keyboard::X, eCellType::Stone);
            spawnMaterial(grid, sf::Keyboard::C, eCellType::Crystal);
            spawnMaterial(grid, sf::Keyboard::F, eCellType::Fire);
        }


        vec2i mouse_pos = getMousePos();
        grid_mouse_coords = grid.convert_coords(mouse_pos, p_window_size); 

        //p_frag_shader.setUniform("u_src_pos", (player->pos - (vec2f)grid.getViewWindow().min) * ((float)WW / grid.getViewWindow().size().x) );

        //imgui
        {
            ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(ImGuiWindowSize.x, ImGuiWindowSize.y), ImGuiCond_FirstUseEver);

            ImGui::Begin("Demo window", nullptr);

            //printing material under cursor
            ImGui::Text("%s", to_str(grid.get(grid_mouse_coords).type) + sizeof("eCellType:"));

            //printing fps
            ImGui::Text("%f", p_avg_fps);
            //show updated segments
            if(ImGui::Button("edit")) {
                editor_mode = !editor_mode;
            }
            if(ImGui::Button("show change")) {
                grid.debug.isActive = !grid.debug.isActive;
            }
            if(grid.debug.isActive) {
                if(ImGui::Button("show updates")) {
                    grid.debug.showUpdated = !grid.debug.showUpdated;
                }
                if(grid.debug.showUpdated) {
                    static float col[3] = {grid.debug.update_clr.r / 255.f, grid.debug.update_clr.g / 255.f, grid.debug.update_clr.b / 255.f};
                    ImGui::ColorEdit3("change update color", col); 
                    grid.debug.update_clr.r = col[0] * 255;
                    grid.debug.update_clr.g = col[1] * 255;
                    grid.debug.update_clr.b = col[2] * 255;
                }
                if(ImGui::Button("show draws")) {
                    grid.debug.showDraws = !grid.debug.showDraws;
                }
                if(grid.debug.showDraws) {
                    static float col[3] = {grid.debug.draw_clr.r / 255.f, grid.debug.draw_clr.g / 255.f, grid.debug.draw_clr.b / 255.f};
                    ImGui::ColorEdit3("change draw color", col); 
                    grid.debug.draw_clr.r = col[0] * 255;
                    grid.debug.draw_clr.g = col[1] * 255;
                    grid.debug.draw_clr.b = col[2] * 255;
                }
            }
            static std::string filename = "test.bin";
            ImGui::Text("filename: \"%s\"", filename.c_str());
            if(ImGui::Button("export")) {
                std::cin >> filename;
                grid.exportToFile(filename.c_str());
            }
            if(ImGui::Button("import")) {
                std::cin >> filename;
                auto tmp_grid = Grid(1, 1);
                tmp_grid.importFromFile(filename.c_str());
                grid.mergeAt(tmp_grid);
            }
            //brush size
            ImGui::SliderInt("brush", &brush_size, 1, 32);
            //material selection
            {
                std::vector<eCellType> all_materials;
                for(auto i = eCellType::Air; i <= eCellType::Bedrock; i = (eCellType)((int)i + 1))
                    all_materials.push_back(i);
                std::vector<const char*> all_materials_str;
                for(auto i : all_materials) {
                    //ohh so haacky
                    all_materials_str.push_back(to_str(i) + sizeof("eCellType:"));
                }

                static int item_current = 0;
                ImGui::ListBox("mat", &item_current, &all_materials_str[0], all_materials_str.size(), 17);
                brush_material = all_materials[item_current];

                on_brush_click = false;
                if(brush_material == eCellType::Seed){
                    on_brush_click = true;
                    brush_size = 1;
                }
            }
            ImGui::End();
        }
    }
    void draw(sf::RenderTarget& rw, const Grid& grid) override {
        g_player_sprite.drawAt({100, 100}, rw, grid);
    }
    bool setup(Grid& grid) override {
        //bindFragShader("assets/light_shader.frag");
        //p_frag_shader.setUniform("u_src_range", 500.f* ((float)WW / GWW));
        //p_frag_shader.setUniform("u_src_intensity", 1.5f);
        brush_size = std::clamp<int>(sqrt(grid.getViewWindow().size().x * grid.getViewWindow().size().y) / 32 / 2, 1, 0xffffff);
        CellVar::addReplicatorMap("tree.bin", 0);
        CellVar::addReplicatorMap("house.bin", 1);
        CellVar::addReplicatorMap("horse.bin", 2);
        CellVar::addReplicatorMap("island.bin", 3);

#define SPAWN_REPLICATOR(cid, cx, cy)\
            auto t = brush_size;\
\
            CellVar cv(eCellType::Replicator);\
            cv.var.Replicator.id = cid;\
            cv.var.Replicator.x= cx;\
            cv.var.Replicator.y= cy;\
\
            brush_size = 1;\
            spawnAtBrush(grid, false, &cv);\
\
            brush_size = t

        addKeyHook(sf::Keyboard::Q, 
            [&]() {
                if(!editor_mode)
                    return;
                auto t = brush_size;
                brush_size = 1;
                spawnMaterial(grid, sf::Keyboard::Q, eCellType::Seed);
                brush_size = t;
            });
        addKeyHook(sf::Keyboard::Num0, 
            [&]() {
                if(!editor_mode)
                    return;
                SPAWN_REPLICATOR(0, 0, 0);
            });
        addKeyHook(sf::Keyboard::Num1, 
            [&]() {
                if(!editor_mode)
                    return;
                SPAWN_REPLICATOR(1, 0, 0);
            });
        addKeyHook(sf::Keyboard::Num2, 
            [&]() {
                if(!editor_mode)
                    return;
                SPAWN_REPLICATOR(2, 0, 0);
            });
        addKeyHook(sf::Keyboard::Num3, 
            [&]() {
                if(!editor_mode)
                    return;
                SPAWN_REPLICATOR(3, 0, 0);
            });
        addKeyHook(sf::Keyboard::R, 
            [&]() {
                if(!editor_mode)
                    return;
                grid = Grid(GWW, GWH);
            });
        addKeyHook(sf::Keyboard::V, 
            [&]() {
                grid.setViewWindow(grid.getDefaultViewWindow());
            });
        addKeyHook(sf::Keyboard::Space, 
            [&]() {
                editor_mode = !editor_mode;
            });
        addKeyHook(sf::Keyboard::Z, 
            [&]() {
                if(last_mouse_pos.x == -1) {
                    last_mouse_pos = getMousePos();
                    last_mouse_pos = grid.convert_coords(last_mouse_pos, p_window_size);
                }
            }, eKeyHookState::isPressed);
        addKeyHook(sf::Keyboard::Z, 
            [&]() {
                AABBi new_view;

                vec2i this_mouse_pos = getMousePos();
                this_mouse_pos= grid.convert_coords(this_mouse_pos, p_window_size);

                new_view.min.x = std::min(this_mouse_pos.x, last_mouse_pos.x);
                new_view.min.y = std::min(this_mouse_pos.y, last_mouse_pos.y);
                new_view.max.x = std::max(this_mouse_pos.x, last_mouse_pos.x);
                new_view.max.y = std::max(this_mouse_pos.y, last_mouse_pos.y);
                int max_size = std::max(new_view.size().x, new_view.size().y);
                new_view.max.x = new_view.min.x + max_size;
                new_view.max.y = new_view.min.y + max_size;

                new_view.min -= vec2i(1, 1);

                grid.setViewWindow(new_view);
                last_mouse_pos.x = -1;
            }, eKeyHookState::isReleased);

        return true;
    }

    Demo() : QuarryApp(vec2i(GWW, GWH), vec2f(WW, WH)) {}
};
int main()
{
    Demo app;
    app.run();
    return 0;
}
