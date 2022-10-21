#pragma once

#include "imgui.h"
#include "core.h"

enum class eKeyHookState {
    isPressed,
    isReleased
};
class QuarryApp {

    sf::Clock ImGuiClock;

    std::chrono::high_resolution_clock::time_point start;
    std::vector<float> fps_array;
    sf::Clock sec_clock;
    float last_avg = 60.f;

    std::map<sf::Keyboard::Key,std::function<void(void)> > pressed_keypress_hooks;
    std::map<sf::Keyboard::Key,std::function<void(void)> > released_keypress_hooks;
    window_t window;

protected:
    bool p_isActive = true;
    bool p_isUpdating = true;

    float p_avg_fps = 60.f;
    float p_deltaTime = 0.f;

    vec2i p_grid_size;
    vec2f p_window_size;

    inline vec2i getMousePos() {
        return sf::Mouse::getPosition(window);
    }

    inline void exit() { p_isActive = false; }
    inline void addKeyHook(sf::Keyboard::Key key, std::function<void(void)> func, eKeyHookState state = eKeyHookState::isPressed) {
        switch(state) {
            case eKeyHookState::isPressed :
                pressed_keypress_hooks[key] = func;
                break;
            case eKeyHookState::isReleased :
                released_keypress_hooks[key] = func;
                break;

        }
    }
public:
    virtual bool setup(Grid& grid) { return true; }
    virtual void update(Grid& grid) {}
    virtual void cleanup() {}
    void run();

    ~QuarryApp() { cleanup(); }
    QuarryApp(vec2i grid_size, vec2f win_resolution);
};
