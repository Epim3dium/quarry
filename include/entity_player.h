#include "entity.h"

class Player : public Entity {
    vec2i* m_move_input;
    bool m_grounded = false;

    //how high above the ground you will 'hover'
    const int leg_length = 10;
    const float repel_leg_vel = 0.1f;
    const float gravity_force = 0.1f;
    const float max_vert_speed = 2.f;

    AABBi m_segment_to_redraw;
public:
    float speed = 0.05f;
    void update(Grid& grid) override {
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

        vec2f last_pos = pos;
        pos += vel;
        pos.x = std::clamp<float>(pos.x, 0.f, grid.getDefaultViewWindow().max.x - 1);
        pos.y = std::clamp<float>(pos.y, 0.f, grid.getDefaultViewWindow().max.y - 1);
        auto& cur_prop = grid.get(pos.x, pos.y).getProperty();
        if(cur_prop.state == eState::Soild || cur_prop.state == eState::Powder) {
            pos = last_pos;
            vel = {0, 0};
        }
    }
    void draw(const AABBi& view_window, Grid& grid, window_t& rw) override {
        grid.m_redrawSegment(m_segment_to_redraw, view_window, rw);
        AABBi aabb_changed;
        auto drawAtOffset = [&](vec2i v, clr_t color) {
            aabb_changed.update_to_contain(vec2i(pos) + v);
            grid.drawCellAt(pos.x + v.x, pos.y + v.y, view_window, color, rw);
        };
        auto skin_color = clr_t(255,219,172);
        drawAtOffset(vec2i(1, 0), skin_color);
        drawAtOffset(vec2i(-1, 0), skin_color);
        drawAtOffset(vec2i(0, 1), skin_color);
        drawAtOffset(vec2i(0, -1), skin_color);
        for(int y = -5; y != -1; y++)
            for(int x = -1; x != 2; x++)
                drawAtOffset({x, y}, clr_t(255, 0, 0));
        vec2i padding(2, 2);
        aabb_changed.max += padding;
        aabb_changed.min -= padding;
        m_segment_to_redraw = aabb_changed;
    }

    Player(vec2i* move_input) : Entity(eEntityType::Humanoid), m_move_input(move_input) 
    {
        vel = {0, 0};
    }
};
