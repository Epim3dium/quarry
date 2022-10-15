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

#define GWW 512
#define GWH 512

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
    void draw(Grid& g) {
        spr.drawAt((vec2i)pos, g);
    }
    void update(Grid& g) {
        rb.update(g, pos);
    }

    vec2f pos;
    Ball() : spr(g_sprite) {}
    Ball(vec2f p) : pos(p), spr(g_sprite) {
        rb.radius = 5.f;
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
        grid.toggleDebug = true;
        addKeyHook(sf::Keyboard::Q, 
            [&]() {
                auto t = brush_size;
                brush_size = 1;
                spawnMaterial(grid, sf::Keyboard::Q, eCellType::Seed);
                brush_size = t;
            });
        addKeyHook(sf::Keyboard::Up, 
            [&]() {
                ball.rb.vel.y = 3;
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
