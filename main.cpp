#include <iostream>

#include <SFML/Graphics.hpp>

#include "imgui.h"
#include "imgui-SFML.h"

#include "utils.h"
#include "grid.h"
#include "timer.h"
#include "grid_sprite.h"

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
    const static GridSprite g_sprite;
    GridSprite spr;
public:
    struct {
        float bounciness = 0.1f;
        float drag = 0.03f;
        size_t raycast_c = 16U;
        float friction = 0.1f;
    }physics;
    void draw(Grid& g) {
        spr.drawAt((vec2i)pos, g);
    }
    void update(Grid& g) {
        auto isCollidable = [&](const CellVar& cv) {
            return cv.getProperty().density > 1000;
        };
        float vl = length(vel);
        vec2f vn = normal(vel);
        if(abs(vel.x) > 0.001f || abs(vel.y) > 0.001f)
            vel -= vn * vl * vl * physics.drag;
        vec2f col_n = {0, 0};
        for(size_t i = 0; i < physics.raycast_c; i++) {
            float ang = ((float)i / physics.raycast_c) * 2.f * 3.141f;
            vec2f dir = vec2f(sinf(ang), cosf(ang));
            vec2f point = pos + dir * radius;
            if(isCollidable(g.get(point.x, point.y)))
                col_n -= dir;
        }
        if(length(col_n) != 0.f) {
            col_n = normal(col_n);
            float d = std::clamp(dot(vel, col_n), -INFINITY, 0.f);

            float a = atan2(col_n.x, col_n.y);
            a -= atan2(vn.x, vn.y);
            a = abs(a);

            auto remap = Remap<float, float>(0.f, 1.f, 1.f, 50.f);

            vel.x -= (1.f + physics.bounciness) * (col_n.x * d); 
            vel.y -= (1.f + physics.bounciness) * (col_n.y * d);

            vel *= 1.f - physics.friction * abs(sin(a));
        }
        pos += vel;
    }

    float radius = 5.0f;
    vec2f vel = {0, 0};
    vec2f pos;
    Ball(vec2f p) : pos(p), spr(g_sprite) {}
};
const GridSprite Ball::g_sprite("./assets/player.png");

int main()
{
    InitializeProperties();

    Grid grid(GWW, GWH);
    grid.setViewWindow(VIEW_WINDOW);
    grid.toggleDebug = true;
    Ball ball(vec2f(50, 50));
    //player.pos = vec2f((float)GWW / 2, 10);
    
    int brush_size = DEFAULT_BRUSH_SIZE;
    bool on_brush_click = false;
    eCellType brush_material = eCellType::Sand;

    sf::RenderWindow window(sf::VideoMode(WW, WH), "automata");
    window.setFramerateLimit(60);

    ImGuiWindowFlags imgui_flags = 0;
    imgui_flags |= ImGuiWindowFlags_NoMove;
    imgui_flags |= ImGuiWindowFlags_NoResize;
    imgui_flags |= ImGuiWindowFlags_NoCollapse;

    ImGui::SFML::Init(window);
    vec2f ImGuiWindowSize(WW/5, WH / 2);

    //setting font to something more readable and bigger
    setupImGuiFont();

    vec2i player_input = {0, 0};
    const auto& spawnAtBrush = [&](bool ifEmpty = false) {
        vec2i mouse_pos = sf::Mouse::getPosition(window);
        if(mouse_pos.x < ImGuiWindowSize.x && mouse_pos.y < ImGuiWindowSize.y)
            return;
        auto grid_mouse_coords = grid.convert_coords(mouse_pos, window); 
        for(int y = -brush_size/2; y < round((float)brush_size/2.f); y++)
            for(int x = -brush_size/2; x < round((float)brush_size/2.f); x++)
                if(grid.inBounds(grid_mouse_coords + vec2i(x, y)) && (grid.get(grid_mouse_coords + vec2i(x, y)).type == eCellType::Air || !ifEmpty))
                    grid.set(grid_mouse_coords + vec2i(x, y), CellVar(brush_material));
    };
    const auto& spawnMaterial = [&](sf::Keyboard::Key k, eCellType type) {
        brush_material = type;
        if(sf::Keyboard::isKeyPressed(k)){
            spawnAtBrush();
        }
    };
    //for calculating fps
    std::chrono::high_resolution_clock::time_point start;
    std::vector<float> fps_array;
    float avg_fps = 60.f;
    sf::Clock sec_clock;
    float last_avg = 60.f;

    //needed to avoid flickering caused by sfml's double buffering
    window.display();

    std::flush(std::cout);
    //for imgui
    sf::Clock ImGuiClock;
    while (window.isOpen()) {
        sf::Event event;
        start = std::chrono::high_resolution_clock::now();
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
            else if(event.type == sf::Event::KeyPressed) {
                if(event.key.code == sf::Keyboard::Q) {
                    auto t = brush_size;
                    brush_size = 1;
                    spawnMaterial(sf::Keyboard::Q, eCellType::Seed);
                    brush_size = t;
                }

                if(event.key.code == sf::Keyboard::Num0)
                    brush_size += 1;

                if(event.key.code == sf::Keyboard::Up) {
                    ball.vel.y = 3;
                }
                if(event.key.code == sf::Keyboard::Num9)
                    brush_size -= 1;
                if(event.key.code == sf::Keyboard::R){
                    grid = Grid(GWW, GWH);
                    window.display();
                    brush_size = DEFAULT_BRUSH_SIZE;
                }
                if(event.key.code == sf::Keyboard::V){
                    grid.setViewWindow(grid.getDefaultViewWindow());
                }
            }
            else if(event.type == sf::Event::MouseButtonPressed) {
                if(on_brush_click)
                    spawnAtBrush();
            }
            ImGui::SFML::ProcessEvent(event);
        }
        ImGui::SFML::Update(window, ImGuiClock.restart());
        //game loop

        if (sf::Mouse::isButtonPressed(sf::Mouse::Left) && !on_brush_click) {
            spawnAtBrush();
        }
        spawnMaterial(sf::Keyboard::A, eCellType::Acid);
        spawnMaterial(sf::Keyboard::O, eCellType::Wood);
        spawnMaterial(sf::Keyboard::D, eCellType::Dirt);
        spawnMaterial(sf::Keyboard::S, eCellType::Sand);
        spawnMaterial(sf::Keyboard::W, eCellType::Water);
        spawnMaterial(sf::Keyboard::L, eCellType::Lava);
        spawnMaterial(sf::Keyboard::X, eCellType::Stone);
        spawnMaterial(sf::Keyboard::C, eCellType::Crystal);
        spawnMaterial(sf::Keyboard::F, eCellType::Fire);


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
            ball.vel.x += (float)player_input.x * player_speed;

        {
            epi::timer::scope timer("grid", true);
            {
                epi::timer::scope timer("draw");
                grid.redrawChangedSegments();
            }
            {
                epi::timer::scope timer("update");
                grid.updateChangedSegments();
            }
        }
        {
            ball.vel.y -= 0.07f;
            ball.update(grid);
            ball.draw(grid);
        }
        window.clear();
        grid.render(window);


        //imgui window
        {
            ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(ImGuiWindowSize.x, ImGuiWindowSize.y), ImGuiCond_FirstUseEver);

            ImGui::Begin("Demo window", nullptr, imgui_flags);

            //printing fps
            ImGui::Text("%f", avg_fps);
            //show updated segments
            if(ImGui::Button("show update seg's")) {
                show_updated_segments = !show_updated_segments;
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
        {
            auto end = std::chrono::high_resolution_clock::now();
            float fps = (float)1e9/(float)std::chrono::duration_cast<std::chrono::nanoseconds>(end-start).count();
            if(fps_array.size() >= 64)
                fps_array.erase(fps_array.begin());
            fps_array.push_back(fps);
            avg_fps = (float)std::accumulate(fps_array.begin(), fps_array.end(), 0) / fps_array.size();

            if(print_fps){
                if(sec_clock.getElapsedTime().asSeconds() > 1.f) {
                    if(last_avg / avg_fps < 0.8f || avg_fps / last_avg < 0.8f)
                        std::cout << "[FPS]: " << avg_fps << "\n";
                    sec_clock.restart();
                    last_avg = avg_fps;
                }
            }
        }
        ImGui::SFML::Render(window);
        window.display();
    }

    return 0;
}
