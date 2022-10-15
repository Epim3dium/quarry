#pragma once

#include "imgui.h"
#include "core.h"

enum class eKeyHookState {
    isPressed,
    isReleased
};
class QuarryApp {
    bool isActive = true;

    sf::Clock ImGuiClock;

    std::chrono::high_resolution_clock::time_point start;
    std::vector<float> fps_array;
    sf::Clock sec_clock;
    float last_avg = 60.f;

    struct KeyHookInfo{
        std::function<void(void)> func;
        eKeyHookState state;
        KeyHookInfo() {}
    };
    std::map<sf::Keyboard::Key, KeyHookInfo> keypress_hooks;
    window_t window;

protected:
    float p_avg_fps = 60.f;
    vec2i p_grid_size;
    vec2f p_window_size;
    ImGuiWindowFlags p_imgui_flags = 0;

    inline vec2i getMousePos() {
        return sf::Mouse::getPosition(window);
    }

    inline void exit() { isActive = false; }
    inline void addKeyHook(sf::Keyboard::Key key, std::function<void(void)> func, eKeyHookState state = eKeyHookState::isPressed) {
        keypress_hooks[key].func = func;
        keypress_hooks[key].state = state;
    }
public:
    virtual bool setup(Grid& grid) { return true; }
    virtual void update(Grid& grid) {}
    virtual void cleanup() {}
    void run();

    ~QuarryApp() { cleanup(); }
    QuarryApp(vec2i grid_size, vec2f win_resolution);
};
