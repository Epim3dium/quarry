#include <iostream>

#include <SFML/Graphics.hpp>

#include "utils.h"
#include "grid.h"
#include "timer.h"
#include "entity_player.h"

#define WW 2048.f
#define WH 2048.f

#define GWW 256
#define GWH 256

#define DEFAULT_BRUSH_SIZE 3


const bool show_updated_segments = true;
const float updated_segments_opacity = 127;
const bool print_fps = true;

int main()
{
    InitializeProperties();

    vec2i player_input = {0, 0};
    std::vector<Player> player_swarm;
    for(int i = 1; i < 10; i++) {
        player_swarm.push_back(&player_input);
        player_swarm.back().pos = vec2f{(float)GWW / 10 * i, 10};
    }

    Grid grid(GWW, GWH);
    //player.pos = vec2f((float)GWW / 2, 10);
    
    int brush_size = DEFAULT_BRUSH_SIZE;
    //body.child = std::make_unique<stem_seg_t>(stem_seg_t({200.f, 200.f}, 0.f, 100.f, &body));

    sf::RenderWindow window(sf::VideoMode(WW, WH), "SFML works!");
    window.setFramerateLimit(60);
    const auto& SpawnIfEmpty = [&](sf::Keyboard::Key k, eCellType type) {
        if(sf::Keyboard::isKeyPressed(k)){
            auto ture_coords = grid.convert_coords(sf::Mouse::getPosition(window), window); 
            for(int y = -brush_size/2; y < round((float)brush_size/2.f); y++)
                for(int x = -brush_size/2; x < round((float)brush_size/2.f); x++)
                    if(grid.inBounds(ture_coords + vec2i(x, y)) && grid.get(ture_coords + vec2i(x, y)).type == eCellType::Air)
                        grid.set(ture_coords + vec2i(x, y), CellVar(type));
        }
    };
    //for calculating fps
    sf::Clock sec_clock;
    std::chrono::high_resolution_clock::time_point start;
    float fps_sum = 0;
    size_t fps_count = 0;

    //needed to avoid flickering caused by sfml's double buffering
    grid.draw(window);
    window.display();
    grid.draw(window);

    while (window.isOpen()) {
        sf::Event event;
        start = std::chrono::high_resolution_clock::now();
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
            if(event.type == sf::Event::KeyPressed) {
                if(event.key.code == sf::Keyboard::Q) {
                    auto t = brush_size;
                    brush_size = 1;
                    SpawnIfEmpty(sf::Keyboard::Q, eCellType::Seed);
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
        }
        //game loop

        if(sf::Keyboard::isKeyPressed(sf::Keyboard::BackSpace)){
            auto ture_coords = grid.convert_coords(sf::Mouse::getPosition(window), window); 
            for(int y = -brush_size/2; y < round((float)brush_size/2.f); y++)
                for(int x = -brush_size/2; x < round((float)brush_size/2.f); x++)
                    if(grid.inBounds(ture_coords + vec2i(x, y)) && grid.get(ture_coords + vec2i(x, y)).type != eCellType::Bedrock)
                        grid.set(ture_coords + vec2i(x, y), CellVar(eCellType::Air));
        }
        SpawnIfEmpty(sf::Keyboard::A, eCellType::Acid);
        SpawnIfEmpty(sf::Keyboard::O, eCellType::Wood);
        SpawnIfEmpty(sf::Keyboard::D, eCellType::Dirt);
        SpawnIfEmpty(sf::Keyboard::S, eCellType::Sand);
        SpawnIfEmpty(sf::Keyboard::W, eCellType::Water);
        SpawnIfEmpty(sf::Keyboard::L, eCellType::Lava);
        SpawnIfEmpty(sf::Keyboard::X, eCellType::Stone);
        SpawnIfEmpty(sf::Keyboard::C, eCellType::Crystal);

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

        if(print_fps){
            auto end = std::chrono::high_resolution_clock::now();
            float fps = (float)1e9/(float)std::chrono::duration_cast<std::chrono::nanoseconds>(end-start).count();
            fps_sum += fps;
            fps_count++;
            if(sec_clock.getElapsedTime().asSeconds() > 3.f) {
                sec_clock.restart();
                std::cerr << "[FPS]: " << fps_sum/fps_count << "\n";
                std::cerr << "[grid]: " << epi::timer::Get("grid").ms() << "\t[player]: " << epi::timer::Get("player").ms() << "\n";
                std::cerr << "{player}: update): " << epi::timer::Get("player_update").ms() << "\tdraw): " << epi::timer::Get("player_draw").ms() << "\n";
                epi::timer::clearTimers();
                fps_sum = 0;
                fps_count = 0;
            }
        }
        window.display();
    }

    return 0;
}
