#include "utils.h"
#include "entity_player.h"

RNG g_rng;

float angle(vec2f a, vec2f b) {
    vec2f d = b - a;
    return std::atan2(d.y, d.x);
}
float length(vec2f a){
    return sqrt(a.x * a.x + a.y * a.y);
}
bool intersects(const AABBi& a, const AABBi& b) {
    return (a.min.x < b.max.x && a.max.x > b.min.x) &&
             (a.min.y < b.max.y && a.max.y > b.min.y);
}
const GridSprite Player::sprite("./assets/player.png");
