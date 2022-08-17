#include "entity.h"
#include "grid_sprite.h"
#include "grid_rigidbody.h"
#include "timer.h"
#include "RNG.h"

class Player : public Entity {
    vec2i* m_move_input;
    bool m_grounded = false;

    //how high above the ground you will 'hover'
    const float repel_leg_vel = 0.1f;
    const float gravity_force = 0.1f;
    const float max_vert_speed = 2.f;
    const vec2i size = vec2i(3, 9);
    static const GridSprite sprite;

    //needed 2 of last pos for both render buffers of sfml to redraw last occupied positions 
    vec2f m_last_pos[2];
    GridSprite m_sprite;
public:
    GridRigidbody m_rigidbody;
    float speed = 0.05f;
    void update(Grid& grid) override;
    void draw(const AABBi& view_window, Grid& grid, window_t& rw) override;

    Player(vec2i* move_input) : Entity(eEntityType::Humanoid), m_move_input(move_input), m_sprite(sprite) 
    {
        static RNG rng;
        clr_t color = rng.Random({clr_t(128, 34, 42), clr_t(37, 94, 36), clr_t(55, 43, 161)});

        for(clr_t* i = m_sprite.getPixels(); i < m_sprite.getPixels() + m_sprite.getWidth() * m_sprite.getHeight() * sizeof(clr_t); i++) {
            if(i->r == 255) {
                *i = color;
            }
        }
        m_rigidbody.size = vec2f(m_sprite.getWidth(), m_sprite.getHeight());
        m_rigidbody.vel = {0, 0};
    }
};
