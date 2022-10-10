#include "entity_player.h"

void Player::update(Grid& grid) {
    //moving last positions back by 1
    m_last_pos[1] = m_last_pos[0];
    m_last_pos[0] = pos;

    if(m_move_input) {
        m_rigidbody.vel.x += float(m_move_input->x) * speed;
    }
    //check for ground under entity
    m_grounded = m_rigidbody.physics.last_side_collided.y == -1;
    if(m_move_input->y == 1 && m_grounded) {
        m_rigidbody.vel.y += jump_force;
    }
    m_rigidbody.update(this, grid);
}
void Player::draw(Grid& grid, window_t& rw) {
    epi::timer::scope timer("player_draw");
    //grouping old pixels that should be cleared
    GridSprite background(size.x, size.y);
    for(int y = 0; y < sprite.getHeight(); y++) {
        for(int x = 0; x < sprite.getWidth(); x++) {
            if(grid.inBounds(vec2i(m_last_pos[1]) + vec2i(x, y) - size / 2 ))
                background.set(x, y, grid.get(vec2i(m_last_pos[1]) + vec2i(x, y) - size / 2).color);
        }
    }
    //drawing pixel buffer
    grid.drawSpriteAt(background, vec2i(m_last_pos[1]),  rw);
    grid.drawSpriteAt(sprite, vec2i(pos),  rw);
}
