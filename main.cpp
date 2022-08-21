#include <iostream>

#include <SFML/Graphics.hpp>

#include "imgui.h"
#include "imgui-SFML.h"

#include "utils.h"
#include "grid.h"
#include "timer.h"
#include "entity_player.h"

#define WW 2048.f
#define WH 2048.f

#define GWW 256 
#define GWH 256 

#define DEFAULT_BRUSH_SIZE (sqrt(GWW * GWH) / 32 / 2)


const bool show_updated_segments = false;
const float updated_segments_opacity = 127;
const bool print_fps = true;

int main()
{
    InitializeProperties();

    vec2i player_input = {0, 0};
    std::vector<Player> player_swarm;
    for(int i = 1; i < 2; i++) {
        player_swarm.push_back(&player_input);
        player_swarm.back().pos = vec2f{(float)GWW / 10 * i, 10};
    }

    Grid grid(GWW, GWH);
    //player.pos = vec2f((float)GWW / 2, 10);
    
    int brush_size = DEFAULT_BRUSH_SIZE;
    eCellType brush_material = eCellType::Sand;

    sf::RenderWindow window(sf::VideoMode(WW, WH), "SFML works!");
    window.setFramerateLimit(60);

    ImGuiWindowFlags imgui_flags = 0;
    imgui_flags |= ImGuiWindowFlags_NoMove;
    imgui_flags |= ImGuiWindowFlags_NoResize;
    imgui_flags |= ImGuiWindowFlags_NoCollapse;

    ImGui::SFML::Init(window);

    //setting font to something more readable and bigger
    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->Clear();
    ImFont* font = io.Fonts->AddFontFromFileTTF("assets/Consolas.ttf", 24.f);
    ImGui::SFML::UpdateFontTexture();

    const auto& spawnAtBrush = [&]() {
        auto grid_mouse_coords = grid.convert_coords(sf::Mouse::getPosition(window), window); 
        for(int y = -brush_size/2; y < round((float)brush_size/2.f); y++)
            for(int x = -brush_size/2; x < round((float)brush_size/2.f); x++)
                if(grid.inBounds(grid_mouse_coords + vec2i(x, y)) && grid.get(grid_mouse_coords + vec2i(x, y)).type == eCellType::Air)
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
    grid.draw(window);
    window.display();
    grid.draw(window);

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
            if(event.key.code == sf::Keyboard::Escape) {
                window.close();
            }
            if(event.type == sf::Event::KeyPressed) {
                if(event.key.code == sf::Keyboard::Q) {
                    auto t = brush_size;
                    brush_size = 1;
                    spawnMaterial(sf::Keyboard::Q, eCellType::Seed);
                    brush_size = t;
                }

                if(event.key.code == sf::Keyboard::Num0)
                    brush_size += 1;

                if(event.key.code == sf::Keyboard::Num9)
                    brush_size -= 1;
                if(event.key.code == sf::Keyboard::R){
                    grid = Grid(GWW, GWH);
                    grid.draw(window);
                    window.display();
                    grid.draw(window);
                    brush_size = DEFAULT_BRUSH_SIZE;
                }
            }
            ImGui::SFML::ProcessEvent(event);
        }
        ImGui::SFML::Update(window, ImGuiClock.restart());
        //game loop

        if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
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
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
            player_input.y = 1;
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
            player_input.y = -1;
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
            player_input.x = -1;
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
            player_input.x = 1;

        {
            epi::timer::scope timer("grid");
            grid.redrawChangedSegment(window);
            grid.updateChangedSegments();
        }
        {
            epi::timer::scope timer("player");
            for(auto& player : player_swarm) {
                player.update(grid);
                player.draw(grid.getDefaultViewWindow(), grid, window);
            }
        }

        if(show_updated_segments){
            clr_t color(255, 0, 255, updated_segments_opacity);
            for(auto& cur : grid.m_ChangedSectors) {
                for(int x = cur.min.x; x < cur.max.x; x++) {
                    int y = cur.max.y - 1;
                    grid.drawCellAt(x, y, color, window); 
                }
                for(int x = cur.min.x; x < cur.max.x; x++) {
                    int y = cur.min.y;
                    grid.drawCellAt(x, y, color, window); 
                }
                for(int y = cur.min.y; y < cur.max.y; y++) {
                    int x = cur.min.x;
                    grid.drawCellAt(x, y, color, window); 
                }
                for(int y = cur.min.y; y < cur.max.y; y++) {
                    int x = cur.max.x - 1;
                    grid.drawCellAt(x, y, color, window); 
                }
            }
        }

        //imgui window
        {
            ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(WW / 5, WH), ImGuiCond_FirstUseEver);

            ImGui::Begin("Demo window", nullptr, imgui_flags);

            //printing fps
            if(print_fps)
                ImGui::Text("%f", avg_fps);
            //brush size
            ImGui::SliderInt("size of brush", &brush_size, 1, 32);
            //material selection
            {
                std::vector<eCellType> all_materials;
                for(auto i = eCellType::Air; i < eCellType::Bedrock; i = (eCellType)((int)i + 1))
                    all_materials.push_back(i);
                std::vector<const char*> all_materials_str;
                for(auto i : all_materials)
                    all_materials_str.push_back(to_str(i));

                static int item_current = 0;
                ImGui::ListBox("materials", &item_current, &all_materials_str[0], all_materials_str.size(), 16);
                brush_material = all_materials[item_current];
            }
            ImGui::End();
        }
        if(print_fps){
            auto end = std::chrono::high_resolution_clock::now();
            float fps = (float)1e9/(float)std::chrono::duration_cast<std::chrono::nanoseconds>(end-start).count();
            if(fps_array.size() >= 64)
                fps_array.erase(fps_array.begin());
            fps_array.push_back(fps);
            avg_fps = (float)std::accumulate(fps_array.begin(), fps_array.end(), 0) / fps_array.size();

            if(sec_clock.getElapsedTime().asSeconds() > 1.f) {
                if(last_avg / avg_fps < 0.8f || avg_fps / last_avg < 0.8f)
                    std::cout << "[FPS]: " << avg_fps << "\n";
                sec_clock.restart();
                last_avg = avg_fps;
            }
        }
        ImGui::SFML::Render(window);
        window.display();
    }

    return 0;
}
