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
        float gravity_force = 0.5f; // force as number
    }physics;

    void update(Entity* processed, Grid& grid) {
        auto& pos = processed->pos;
        vel.y -= physics.gravity_force;
        pos += vel;

        vel.x *= 1.f - physics.hor_drag;
        vel.y *= 1.f - physics.ver_drag;

        auto isCollidable = [&](int x, int y) {
            return grid.get(x, y).getProperty().state == eState::Powder || grid.get(x, y).getProperty().state == eState::Soild;
        };
        bool noIntersect = false;
        while(!noIntersect) {
            //up
            for(float x = pos.x - size.x/2 + 1; x < pos.x + size.x / 2; x++) {
                if(isCollidable(x, pos.y + size.y / 2)){
                    pos.y -= 1;
                    goto END_OF_CHECKS;
                }
            }
            //bottom
            for(float x = pos.x - size.x/2 + 1; x < pos.x + size.x / 2; x++) {
                if(isCollidable(x, pos.y - size.y / 2)){
                    pos.y += 1;
                    goto END_OF_CHECKS;
                }
            }
            //left side
            for(float y = pos.y - size.y/2 + 1; y < pos.y + size.y / 2; y++) {
                if(isCollidable(pos.x - size.x / 2, y)){
                    std::cout << "left";
                    pos.x +=1;
                    goto END_OF_CHECKS;
                }
            }
            //right side
            for(float y = pos.y - size.y/2 + 1; y < pos.y + size.y / 2; y++) {
                if(isCollidable(pos.x + size.x / 2, y)){
                    pos.x -= 1;
                    goto END_OF_CHECKS;
                }
            }
            noIntersect = true;
END_OF_CHECKS:;
        }
    }

};
