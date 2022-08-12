#include "entity.h"
#include "timer.h"

class Player : public Entity {
    vec2i* m_move_input;
    bool m_grounded = false;

    //how high above the ground you will 'hover'
    const int leg_length = 10;
    const float repel_leg_vel = 0.1f;
    const float gravity_force = 0.1f;
    const float max_vert_speed = 2.f;

    //needed 2 of last pos for both render buffers of sfml to redraw last occupied positions 
    vec2f m_last_pos[2];
public:
    float speed = 0.05f;
    void update(Grid& grid) override {
        epi::timer::scope timer("player_update");
        //moving last positions back by 1
        m_last_pos[1] = m_last_pos[0];
        m_last_pos[0] = pos;

        if(m_move_input) {
            this->vel.x += float(m_move_input->x) * speed;
        }
        //check for ground under entity
        m_grounded = false;
        for(int i = this->pos.y; i > this->pos.y - leg_length && i >= 0; i--) {
            auto& cur_prop = grid.get(this->pos.x, i).getProperty();
            if(cur_prop.state == eState::Soild || cur_prop.state == eState::Powder) {
                this->vel.y = std::clamp<float>(vel.y, 0, INFINITY);
                this->pos.y = i + leg_length;
                m_grounded = true;
                break;
            }
        }
        if(m_move_input->y == 1 && m_grounded) {
            vel.y += 10.f;
        }
        vel.y -= gravity_force;
        vel.y = std::clamp<float>(vel.y, -max_vert_speed, max_vert_speed);

        vel *= 0.95f;

        pos += vel;
        pos.x = std::clamp<float>(pos.x, 0.f, grid.getDefaultViewWindow().max.x - 1);
        pos.y = std::clamp<float>(pos.y, 0.f, grid.getDefaultViewWindow().max.y - 1);
        auto& cur_prop = grid.get(pos.x, pos.y).getProperty();
        if(cur_prop.state == eState::Soild || cur_prop.state == eState::Powder) {
            pos = m_last_pos[0];
            vel = {0, 0};
        }
    }
    void draw(const AABBi& view_window, Grid& grid, window_t& rw) override {
        epi::timer::scope timer("player_draw");
        //grouping pixels needed to be displayed
        std::vector<std::pair<vec2i, clr_t>> pixels;
        //grouping old pixels that should be cleared
        std::vector<std::pair<vec2i, clr_t>> dark_pixels;
        auto drawAtOffset = [&](vec2i v, clr_t color) {
            auto t = vec2i(m_last_pos[1].x + v.x, m_last_pos[1].y + v.y);
            pixels.push_back(std::make_pair(vec2i(pos) + v, color));
            if(grid.inBounds(t) && t != vec2i(pos) + v)
                dark_pixels.push_back(std::make_pair(t, grid.get(t).color));
        };
        auto skin_color = clr_t(255,219,172);
        //drawing 'head'
        drawAtOffset(vec2i(1, 0), skin_color);
        drawAtOffset(vec2i(-1, 0), skin_color);
        drawAtOffset(vec2i(0, 1), skin_color);
        drawAtOffset(vec2i(0, -1), skin_color);

        //drawing 'body'
        for(int y = -5; y != -1; y++)
            for(int x = -1; x != 2; x++)
                drawAtOffset({x, y}, clr_t(255, 0, 0));
        //drawing pixel buffer
        grid.drawCellVecAt(dark_pixels, view_window, rw);
        grid.drawCellVecAt(pixels, view_window, rw);
    }

    Player(vec2i* move_input) : Entity(eEntityType::Humanoid), m_move_input(move_input) 
    {
        vel = {0, 0};
    }
};
