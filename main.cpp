#include <iostream>

#include <SFML/Graphics.hpp>

#include "imgui.h"
#include "imgui-SFML.h"

#include "utils.h"
#include "core.h"
#include "timer.h"
#include "sprite.h"
#include "rigidbody.h"
#include "quarry.h"

#define WW 2048.f
#define WH 2048.f

#define GWW 1024
#define GWH 1024

#define VIEW_WINDOW AABBi{{-1, -1}, {256, 256}}

class Ball {
    const static QuarrySprite g_sprite;
public:
    QuarrySprite spr;
    CircleRigidbody rb;
    //in degrees
    float grounded_check_angle = 35.f;
    size_t grounded_check_count = 5;

    vec2f pos;
    bool isGrounded = true;
    bool isSemiGrounded = true;

    void draw(Grid& g) {
        spr.drawAt((vec2i)pos, g);
    }
    void update(Grid& g) {
        rb.update(g, pos);
        isGrounded = false;
        isSemiGrounded = false;
        for(float a = -grounded_check_angle / 2.f; a < grounded_check_angle; a += grounded_check_angle / grounded_check_count) {
            float rad = a / 360.f * 2.f * 3.141f;
            auto cur_check = g.get(pos.x + sin(rad) * rb.radius * 1.5f, pos.y - cos(rad) * rb.radius * 1.5f);
            if(cur_check.getProperty().state == eState::Powder || cur_check.getProperty().state == eState::Soild)
                isGrounded = true;
        }
        if(g.get((vec2i)pos).getProperty().state == eState::Liquid)
            isSemiGrounded = true;
    }

    Ball() : spr(g_sprite) {}
    Ball(vec2f p) : pos(p), spr(g_sprite) {
        rb.radius = roundf(spr.getHeight() / 2.f);
        rb.physics.density = 900;
    }
};
const QuarrySprite Ball::g_sprite("./assets/player.png");

class Demo : public QuarryApp {
public:
    Ball ball = Ball(vec2f(50, 50));

    int brush_size;
    bool on_brush_click = false;
    eCellType brush_material = eCellType::Sand;
    vec2i last_mouse_pos = {-1, -1};

    vec2f ImGuiWindowSize = vec2f(WW/5, WH / 2);

    vec2i player_input = {0, 0};

    void spawnAtBrush(Grid& grid, bool ifEmpty = false, const CellVar* cv = nullptr ) {
        vec2i mouse_pos = getMousePos();
        if(mouse_pos.x < ImGuiWindowSize.x && mouse_pos.y < ImGuiWindowSize.y)
            return;
        auto grid_mouse_coords = grid.convert_coords(mouse_pos, p_window_size); 
        for(int y = -brush_size/2; y < round((float)brush_size/2.f); y++)
            for(int x = -brush_size/2; x < round((float)brush_size/2.f); x++) {
                if(grid.inBounds(grid_mouse_coords + vec2i(x, y)) && (grid.get(grid_mouse_coords + vec2i(x, y)).type == eCellType::Air || !ifEmpty)){
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

        if (sf::Mouse::isButtonPressed(sf::Mouse::Left) && !on_brush_click) {
            spawnAtBrush(grid);
        }
        spawnMaterial(grid, sf::Keyboard::A, eCellType::Acid);
        spawnMaterial(grid, sf::Keyboard::O, eCellType::Wood);
        spawnMaterial(grid, sf::Keyboard::D, eCellType::Dirt);
        spawnMaterial(grid, sf::Keyboard::S, eCellType::Sand);
        spawnMaterial(grid, sf::Keyboard::W, eCellType::Water);
        spawnMaterial(grid, sf::Keyboard::L, eCellType::Lava);
        spawnMaterial(grid, sf::Keyboard::X, eCellType::Stone);
        spawnMaterial(grid, sf::Keyboard::C, eCellType::Crystal);
        spawnMaterial(grid, sf::Keyboard::F, eCellType::Fire);
        player_input = {0, 0};
        float player_speed = 0.05f;
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
            player_input.y = 1;
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
            player_input.y = -1;
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
            player_input.x = -1;
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
            player_input.x = 1;
        if(player_input.x != 0 || player_input.y != 0)
            ball.rb.vel.x += (float)player_input.x * player_speed;

        ball.rb.vel.y -= 0.07f;
        ball.update(grid);
        ball.draw(grid);
        {
            ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(ImGuiWindowSize.x, ImGuiWindowSize.y), ImGuiCond_FirstUseEver);

            ImGui::Begin("Demo window", nullptr, p_imgui_flags);

            //printing material under cursor
            vec2i mouse_pos = getMousePos();
            vec2i grid_mouse_coords = grid.convert_coords(mouse_pos, p_window_size); 
            ImGui::Text("%s", to_str(grid.get(grid_mouse_coords).type) + sizeof("eCellType:"));

            //printing fps
            ImGui::Text("%f", p_avg_fps);
            //show updated segments
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
            static std::string filename = "test";
            ImGui::InputText("filename", &filename[0], filename.capacity());
            if(ImGui::Button("export")) {
                grid.exportToFile(filename.c_str());
            }
            if(ImGui::Button("import")) {
                auto tmp_grid = Grid(1, 1);
                tmp_grid.importFromFile(filename.c_str());
                grid.mergeAt(tmp_grid);
            }
            //brush size
            ImGui::SliderInt("brush", &brush_size, 1, 32);
            //material selection
            {
                std::vector<eCellType> all_materials;
                for(auto i = eCellType::Air; i < eCellType::Bedrock; i = (eCellType)((int)i + 1))
                    all_materials.push_back(i);
                std::vector<const char*> all_materials_str;
                for(auto i : all_materials) {
                    //ohh so haacky
                    all_materials_str.push_back(to_str(i) + sizeof("eCellType:"));
                }

                static int item_current = 0;
                ImGui::ListBox("mat", &item_current, &all_materials_str[0], all_materials_str.size(), 16);
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
    bool setup(Grid& grid) override {
        grid.setViewWindow(VIEW_WINDOW);
        brush_size = (sqrt(grid.getViewWindow().size().x * grid.getViewWindow().size().y) / 32 / 2);
        addKeyHook(sf::Keyboard::Q, 
            [&]() {
                auto t = brush_size;
                brush_size = 1;
                spawnMaterial(grid, sf::Keyboard::Q, eCellType::Seed);
                brush_size = t;
            });
        addKeyHook(sf::Keyboard::Up, 
            [&]() {
                if(ball.isGrounded)
                    ball.rb.vel.y = 3.f;
                if(ball.isSemiGrounded)
                    ball.rb.vel.y += 1.f;
            });
        addKeyHook(sf::Keyboard::R, 
            [&]() {
                grid = Grid(GWW, GWH);
                brush_size = (sqrt(grid.getViewWindow().size().x * grid.getViewWindow().size().y) / 32 / 2);
            });
        addKeyHook(sf::Keyboard::V, 
            [&]() {
                grid.setViewWindow(grid.getDefaultViewWindow());
                brush_size = (sqrt(grid.getViewWindow().size().x * grid.getViewWindow().size().y) / 32 / 2);
            });
        addKeyHook(sf::Keyboard::Space, 
            [&]() {
                p_isUpdating = !p_isUpdating;
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
                brush_size = (sqrt(grid.getViewWindow().size().x * grid.getViewWindow().size().y) / 32 / 2);
                last_mouse_pos.x = -1;
            }, eKeyHookState::isReleased);

        return true;
    }

    Demo() : QuarryApp(vec2i(GWW, GWH), vec2f(WW, WH)) {}
};
int main()
{
    /*
            //else if(event.type == sf::Event::MouseButtonPressed) {
            //    if(on_brush_click)
            //        spawnAtBrush();
        if (sf::Mouse::isButtonPressed(sf::Mouse::Left) && !on_brush_click) {
            spawnAtBrush();
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::C)) {
            CellVar cv(eCellType::CrumblingStone);
            cv.var.CrumblingStone.lvl = 1;
            spawnAtBrush(false, &cv);
        }
        */
    Demo app;
    app.run();
    return 0;
}
