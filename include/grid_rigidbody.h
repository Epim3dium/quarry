#pragma once
#include "utils.h"
#include "grid.h"
#include "entity.h"

class GridRigidbody {
public:
    vec2f vel;
    vec2f size;
    struct {
        float hor_drag = 0.1f; // scale 0 - 1
        float ver_drag = 0.1f; // scale 0 - 1
        float gravity_force = 0.3f; // force as number
        vec2i last_side_collided = {0, 0};
    }physics;

    void update(Entity* processed, Grid& grid) {
        physics.last_side_collided = {0, 0};

        auto& pos = processed->pos;
        vel.y -= physics.gravity_force;
        pos += vel;

        vel.x *= 1.f - physics.hor_drag;
        vel.y *= 1.f - physics.ver_drag;

        auto isCollidable = [&](int x, int y) {
            return grid.get(x, y).getProperty().state == eState::Powder || grid.get(x, y).getProperty().state == eState::Soild;
        };
        int check_count = 0;
        const static int max_nb_of_checks = 16;
        bool noIntersect = false;
        while(!noIntersect && check_count < max_nb_of_checks) {
            check_count++;
            //bottom
            for(float x = pos.x - size.x/2 + 1; x < pos.x + size.x / 2; x++) {
                if(isCollidable(x, pos.y - size.y / 2)){
                    pos.y += 1;
                    physics.last_side_collided = {0, -1};
                    goto END_OF_CHECKS;
                }
            }
            //up
            for(float x = pos.x - size.x/2 + 1; x < pos.x + size.x / 2; x++) {
                if(isCollidable(x, pos.y + size.y / 2)){
                    pos.y -= 1;
                    physics.last_side_collided = {0, 1};
                    goto END_OF_CHECKS;
                }
            }
            //left side
            for(float y = pos.y - size.y/2 + 1; y < pos.y + size.y / 2; y++) {
                if(isCollidable(pos.x - size.x / 2, y)){
                    pos.x +=1;
                    physics.last_side_collided = {-1, 0};
                    goto END_OF_CHECKS;
                }
            }
            //right side
            for(float y = pos.y - size.y/2 + 1; y < pos.y + size.y / 2; y++) {
                if(isCollidable(pos.x + size.x / 2, y)){
                    pos.x -= 1;
                    physics.last_side_collided = {1, 0};
                    goto END_OF_CHECKS;
                }
            }
            noIntersect = true;
END_OF_CHECKS:;
        }
    }

};
