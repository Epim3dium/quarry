#include "quarry.h"
#include "imgui.h"
#include "imgui-SFML.h"

#define CONSOLAS_PATH "assets/Consolas.ttf"
static void setupImGuiFont() {
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
void QuarryApp::run() {
    Grid grid(p_grid_size.x, p_grid_size.y);
    if(!setup(grid))
        return;
    while (window.isOpen() && p_isActive) {
        sf::Event event;
        start = std::chrono::high_resolution_clock::now();
        p_window_size = window.getView().getSize();
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
            else if(event.type == sf::Event::KeyPressed) {
                if(pressed_keypress_hooks.find(event.key.code) != pressed_keypress_hooks.end())
                    pressed_keypress_hooks[event.key.code]();
            }
            else if(event.type == sf::Event::KeyReleased) {
                if(released_keypress_hooks.find(event.key.code) != released_keypress_hooks.end())
                    released_keypress_hooks[event.key.code]();
            }
            else if(event.type == sf::Event::KeyReleased) {
            }
            else if(event.type == sf::Event::MouseButtonPressed) {
            }
            ImGui::SFML::ProcessEvent(event);
        }
        //game loop

        grid.redrawChangedSegments();

        ImGui::SFML::Update(window, ImGuiClock.restart());
        update(grid);

        if(p_isUpdating) {
            grid.updateChangedSegments();
        }
        grid.render(window);

        ImGui::SFML::Render(window);
        {
            auto end = std::chrono::high_resolution_clock::now();
             
            float fps = (float)1e9/(float)std::chrono::duration_cast<std::chrono::nanoseconds>(end-start).count();

            if(fps_array.size() >= 64)
                fps_array.erase(fps_array.begin());
            fps_array.push_back(fps);
            p_avg_fps = (float)std::accumulate(fps_array.begin(), fps_array.end(), 0) / fps_array.size();
        }
        auto end = std::chrono::high_resolution_clock::now();
        p_deltaTime = std::chrono::duration_cast<std::chrono::seconds>(end-start).count();

        window.display();
    }
}
QuarryApp::QuarryApp(vec2i grid_size_, vec2f win_resolution) 
    : window(sf::VideoMode(win_resolution.x, win_resolution.y), "QuarryApp"), p_grid_size(grid_size_), p_window_size(win_resolution)
{ 
    CellVar::InitializeProperties();
    window.setFramerateLimit(60);

    ImGui::SFML::Init(window);
    setupImGuiFont();
}
