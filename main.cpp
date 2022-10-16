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

#define DEFAULT_BRUSH_SIZE (sqrt(GWW * GWH) / 32 / 2)

#define VIEW_WINDOW {{0, 0}, {256, 256}}


bool show_updated_segments = false;
unsigned char updated_segments_opacity = 255;
bool print_fps = false;

#define CONSOLAS_PATH "assets/Consolas.ttf"

void setupImGuiFont() {
    sf::Font consolas;
    if (!consolas.loadFromFile(CONSOLAS_PATH)) {
        std::cout << "error while lodaing font file!";
        exit(1);
    }

    //for bigger font
    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->Clear();
    io.WantCaptureMouse = true;
    io.WantCaptureMouseUnlessPopupClose = true;
    
    ImFont* font = io.Fonts->AddFontFromFileTTF(CONSOLAS_PATH, 24.f);
    ImGui::SFML::UpdateFontTexture();
}

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

    int brush_size = DEFAULT_BRUSH_SIZE;
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
        //ball.draw(grid);
        {
            ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(ImGuiWindowSize.x, ImGuiWindowSize.y), ImGuiCond_FirstUseEver);

            ImGui::Begin("Demo window", nullptr, p_imgui_flags);

            //printing fps
            ImGui::Text("%f", p_avg_fps);
            //show updated segments
            if(ImGui::Button("show update seg's")) {
                grid.toggleDebug = !grid.toggleDebug;
            }
            if(show_updated_segments) {
                static float opacity = 1.f;
                ImGui::SliderFloat("opacity", &opacity, 0.f, 1.f);
                updated_segments_opacity = 255 * opacity;
            }
            //brush size
            ImGui::SliderInt("brush", &brush_size, 1, 32);
            //material selection
            {
                std::vector<eCellType> all_materials;
                for(auto i = eCellType::Air; i < eCellType::Bedrock; i = (eCellType)((int)i + 1))
                    all_materials.push_back(i);
                std::vector<const char*> all_materials_str;
                for(auto i : all_materials)
                    all_materials_str.push_back(to_str(i));

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
                brush_size = DEFAULT_BRUSH_SIZE;
            });
        addKeyHook(sf::Keyboard::V, 
            [&]() {
                grid.setViewWindow(grid.getDefaultViewWindow());
            });
        addKeyHook(sf::Keyboard::Space, 
            [&]() {
                if(last_mouse_pos.x == -1) {
                    last_mouse_pos = getMousePos();
                    last_mouse_pos = grid.convert_coords(last_mouse_pos, p_window_size);
                }
            }, eKeyHookState::isPressed);
        addKeyHook(sf::Keyboard::Space, 
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

                grid.setViewWindow(new_view);
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
