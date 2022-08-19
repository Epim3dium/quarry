#include "cell.h"
#include "grid.h"
#include <random>

std::map<eCellType, CellConstants> CellVar::properties = {};

void InitializeProperties() {
    CellVar::properties[eCellType::Air] = CellConstants(
        //powdery
        eState::Gas,
        //density
        0, 
        //flammability
        0.f,
        //behaviour
        [](vec2i v, Grid& grid) {
            return;
        },
        //colors
        {clr_t(10, 20, 50)}
    );
#define MAX_SMOKE_UPDATES (10 * 60)
    CellVar::properties[eCellType::Smoke] = CellConstants(
        //powdery
        eState::Gas,
        //density
        -10,
        //flammability
        0.f,
        //behaviour
        [&](vec2i v, Grid& grid) {
            const auto is_swappable = [&](const int& otherx, const int& othery) {
                auto& my_prop = CellVar::properties[grid.get(v).type]; 
                auto& other_prop = CellVar::properties[grid.get({otherx, othery}).type]; 
                return other_prop.state == eState::Gas && other_prop.density > my_prop.density;
            };
            const auto swap_and_advance_move_counter = [&](const vec2i& me, const vec2i& other) {
                if(is_swappable(other.x, other.y)) {
                    auto t = grid.get(me);
                    t.var.Gas.move_count++;
                    grid.set(me, t);
                    grid.swap_at(me, other);
                    return true;
                }
                return false;
            };
            if(grid.get(v).var.Gas.move_count >= MAX_SMOKE_UPDATES) {
                grid.set(v, eCellType::Air);
            }
            if(swap_and_advance_move_counter(v, {v.x, v.y + 1}))
                return;
            {
                int first_side_down = g_rng.Random() > 0.5f ? 1 : -1; 
                if(swap_and_advance_move_counter(v, {v.x + first_side_down, v.y + 1}))
                    return;
                if(swap_and_advance_move_counter(v, {v.x - first_side_down, v.y + 1}))
                    return;
            }
            {
                int first_side = g_rng.Random() > 0.5f ? 1 : -1; 
                if(swap_and_advance_move_counter(v, {v.x + first_side, v.y}))
                    return;
                if(swap_and_advance_move_counter(v, {v.x - first_side, v.y}))
                    return;
            }
        },
        //colors
        {clr_t(130, 130, 130) }
    );
#define bSTEAM_CONDENSATES false
#define STEAM_CONDENSATION_CHANCE 0.001f
#define STEAM_MIN_AGE_TO_CONDENSATE 90
    CellVar::properties[eCellType::Steam] = CellConstants(
        //powdery
        eState::Gas,
        //density
        -20,
        //flammability
        0.f,
        //behaviour
        [](vec2i v, Grid& grid) {
#if bSTEAM_CONDENSATES
            if(grid.get(v).age >= STEAM_MIN_AGE_TO_CONDENSATE && g_rng.Random() < STEAM_CONDENSATION_CHANCE) {
                grid.set(v, CellVar(eCellType::Water));
                return;
            }
#endif
            CellVar::properties[eCellType::Smoke].update_behaviour(v, grid);
        },
        //colors
        {clr_t(255, 255, 255) }
    );
    CellVar::properties[eCellType::Sand] = CellConstants(
        //powdery
        eState::Powder,
        //density
        1400,
        //flammability
        0.f,
        //behaviour
        [](vec2i v, Grid& grid) {
            const auto is_swappable = [&](const int& otherx, const int& othery) {
                auto& my_prop = CellVar::properties[grid.get(v).type]; 
                auto& other_prop = CellVar::properties[grid.get({otherx, othery}).type]; 
                return (other_prop.state == eState::Liquid || other_prop.state == eState::Gas) || (other_prop.state == eState::Powder  && other_prop.density < my_prop.density);
            };
            if(is_swappable(v.x, v.y - 1)) {
                grid.swap_at(v, {v.x, v.y -1});
                return;
            }
            int first_side = g_rng.Random() > 0.5f ? 1 : -1; 
            if(is_swappable(v.x + first_side, v.y - 1))
                grid.swap_at(v, {v.x + first_side, v.y - 1});
            else if(is_swappable(v.x - first_side, v.y - 1))
                grid.swap_at(v, {v.x - first_side, v.y - 1});
            return;
        },
        //colors
        {clr_t(220, 220, 80), clr_t(250, 250, 40) }
    );
    CellVar::properties[eCellType::Cobblestone] = CellConstants(
        //powdery
        eState::Powder,
        //density
        2300,
        //flammability
        0.f,
        //behaviour
        CellVar::properties[eCellType::Sand].update_behaviour,
        //colors
        {clr_t(150, 150, 150), clr_t(100, 100, 100) }, 
        { {eCellType::Water,    {eCellType::Dirt, 0.01f} } }
    );
#define DIRT_TO_GRASS_REQ_TIME 120
    CellVar::properties[eCellType::Dirt] = CellConstants(
        //powdery
        eState::Powder,
        //density
        1500,
        //flammability
        0.f,
        //behaviour
        [](vec2i v, Grid& grid) {
            //if dirt didnt grow grass already
            if(!grid.get(v).var.Dirt.bgrew_grass) {
                //check if it is old enough
                if(grid.get(v).var.Dirt.grass_timer >= DIRT_TO_GRASS_REQ_TIME) {
                    if(grid.get({v.x, v.y + 1}).type == eCellType::Air) {
                        //grow grass
                        CellVar t(eCellType::Grass);
                        t.var.Grass.down_timer_len = g_rng.Random() > 0.5f ? 1 : 3;
                        grid.set({v.x, v.y + 1}, t);

                    }
                    //not to grow grass endlessly
                    auto me = grid.get(v);
                    me.var.Dirt.bgrew_grass = true;
                    grid.set(v, me);
                } else {
                    auto me = grid.get(v);
                    me.var.Dirt.grass_timer += 1;
                    grid.set(v, me);
                }
            }
            CellVar::properties[eCellType::Cobblestone].update_behaviour(v, grid);
        },
        //colors
        {clr_t(120, 90, 15), clr_t(100, 80, 10) },
        { {eCellType::Lava, { eCellType::Cobblestone, 0.01f } } }
    );
    CellVar::properties[eCellType::Crystal] = CellConstants(
        //powdery
        eState::Powder,
        //density
        2650,
        //flammability
        0.f,
        //behaviour
        [](vec2i v, Grid& grid) {
            auto t = grid.get(v);
            auto& side = t.var.Crystal.CurSide;
            if(abs(side) != 1) {
                t.var.Crystal.CurSide = g_rng.Random() > 0.5f ? 1 : -1;
                grid.set(v, t);
            }
            if( grid.get(v.x + side, v.y - 1).type == eCellType::Crystal) 
            {
                auto& parent = grid.get(v.x + side, v.y - 1);
                if(parent.color != t.color) {
                    t.color = parent.color;
                    grid.set(v, t);
                }
                //to not allow alterations[flickering]
                if( grid.get(v.x - side, v.y - 1).type == eCellType::Crystal)
                    return;
                if(parent.var.Crystal.CurSide != side) {
                    side = parent.var.Crystal.CurSide;
                    t.color = parent.color;
                    grid.set(v, t);
                }
            }else {
                CellVar::properties[eCellType::Sand].update_behaviour(v, grid);
            }
        },
        //colors
        {clr_t(116, 160, 250), clr_t(150, 190, 250), clr_t(120, 120, 220)}
    );
    CellVar::properties[eCellType::Stone] = CellConstants(
        //powdery
        eState::Soild,
        //density
        3000,
        //flammability
        0.f,
        //behaviour
        [](vec2i v, Grid& grid) {
            return;
        },
        //colors
        {clr_t(70, 70, 70)}
    );
    //liquids keep eating framerate so i need a limiter
#define MAX_LIQUID_UPDATES (60 * 10)
    CellVar::properties[eCellType::Water] = CellConstants(
        //powdery
        eState::Liquid,
        //density
        1000,
        //flammability
        0.f,
        //behaviour
        [](vec2i v, Grid& grid) {
            const auto is_swappable = [&](const int& otherx, const int& othery) {
                auto& my_prop = CellVar::properties[grid.get(v).type]; 
                auto& other_prop = CellVar::properties[grid.get({otherx, othery}).type]; 
                return (other_prop.state == eState::Liquid || other_prop.state == eState::Gas) && other_prop.density < my_prop.density;
            };
            const auto swap_and_advance_move_counter = [&](const vec2i& me, const vec2i& other) {
                if(is_swappable(other.x, other.y)) {
                    auto t = grid.get(me);
                    t.var.Liquid.move_count++;
                    grid.set(me, t);
                    grid.swap_at(me, other);
                    return true;
                }
                return false;
            };
            if(grid.get(v).var.Liquid.move_count >= MAX_LIQUID_UPDATES){
                grid.set(v, eCellType::Air);
                return;
            }
            if(swap_and_advance_move_counter(v, {v.x, v.y - 1}))
                return;
            {
                int first_side_down = g_rng.Random() > 0.5f ? 1 : -1; 
                if(swap_and_advance_move_counter(v, {v.x + first_side_down, v.y - 1}))
                    return;
                if(swap_and_advance_move_counter(v, {v.x - first_side_down, v.y - 1}))
                    return;
            }
            {
                int first_side = g_rng.Random() > 0.5f ? 1 : -1; 
                if(swap_and_advance_move_counter(v, {v.x + first_side, v.y})) 
                    return;
                if(swap_and_advance_move_counter(v, {v.x - first_side, v.y})) 
                    return;
            }
        },
        //colors
        {clr_t(50, 50, 150)},
        //reaction with lava -> change to smoke
        { {eCellType::Lava, eCellType::Steam} }
    );

    CellVar::properties[eCellType::Lava] = CellConstants(
        //powdery
        eState::Liquid,
        //density
        2000,
        //flammability
        0.f,
        //behaviour
        CellVar::properties[eCellType::Water].update_behaviour,
        {clr_t(220, 90, 30)},
        //reaction with water -> change to stone
        { {eCellType::Water, eCellType::Cobblestone} });

#define ACID_REACTION_PROBABILITY 0.05f
    CellVar::properties[eCellType::Acid] = CellConstants(
        //powdery
        eState::Liquid,
        //density
        1000,
        //flammability
        0.f,
        //behaviour
        CellVar::properties[eCellType::Water].update_behaviour,
        {clr_t(150, 250, 40)}
    );

    CellVar::properties[eCellType::Grass] = CellConstants(
        //powdery
        eState::Powder,
        //density
        256,
        //flammability
        0.5f,
        //behaviour
        [](vec2i v, Grid& grid) {
            if(grid.get(v).var.Grass.down_timer_len > 0) {
                auto t = grid.get(v);
                t.var.Grass.down_timer_len -= 1;
                grid.set(v, t);
                if(grid.get(v.x, v.y + 1).getProperty().state == eState::Gas)
                    grid.set(v.x, v.y + 1, t);
            }
            if(grid.get(v.x, v.y - 1).getProperty().state == eState::Gas || grid.get(v.x, v.y - 1).getProperty().state == eState::Liquid) {
                grid.swap_at(v, {v.x, v.y - 1});
            }
            if(grid.get(v.x, v.y - 1).type != eCellType::Grass)
                CellVar::properties[eCellType::Sand].update_behaviour(v, grid);
        },
        {clr_t(30, 120, 30)}
    );
#define TREE_MIN_BRANCH_SIZE 5
#define TREE_MAX_BRANCH_SIZE 10
#define TREE_BRANCH_GROW_CHANCE 0.5f
#define TREETOP_RADIUS 5

    CellVar::properties[eCellType::Seed] = CellConstants(
        //powdery
        eState::Powder,
        //density
        256,
        //flammability
        1.f,
        //behaviour
        [](vec2i v, Grid& grid) {
            if(grid.get(v.x, v.y - 1).type == eCellType::Grass) {
                CellVar t(eCellType::Wood);
                t.var.Wood.Dir.y = 1;
                t.var.Wood.Dir.x = 0;
                t.var.Wood.branch_count = 0;
                t.var.Wood.down_timer_len = g_rng.Random<unsigned char>(TREE_MIN_BRANCH_SIZE, TREE_MAX_BRANCH_SIZE) + TREE_MAX_BRANCH_SIZE;
                grid.set(v.x, v.y, t);
            }
            else {
                CellVar::properties[eCellType::Sand].update_behaviour(v, grid);
            }
        },
        //colors
        {clr_t(10, 100, 10)}
    );
    CellVar::properties[eCellType::Wood] = CellConstants(
        //powdery
        eState::Soild,
        //density
        600,
        //flammability
        0.02f,
        //behaviour
        [](vec2i v, Grid& grid) {
            auto& me = grid.get(v);

            //if it is still growing
            //remember that 0 is supposed to always be 'balanced' [aka not updating] state
            if(me.var.Wood.down_timer_len > 0) {
                const auto is_replaceable = [&](const int& otherx, const int& othery) {
                    auto& my_prop = CellVar::properties[grid.get(v).type]; 
                    auto& other_prop = CellVar::properties[grid.get({otherx, othery}).type]; 
                    return grid.get(otherx, othery).type == eCellType::Air;
                };

                if(is_replaceable(v.x + me.var.Wood.Dir.x, v.y + me.var.Wood.Dir.y)) {
                    CellVar t(eCellType::Wood);
                    t.var.Wood.Dir = me.var.Wood.Dir;
                    //since 0 is saved for balanced state -1 is for growing leaves
                    //if it is supposed to reach 0 change it to -1 to grow leaves at the end of each branch
                    t.var.Wood.down_timer_len = (me.var.Wood.down_timer_len - 1 == 0 ? -1 : me.var.Wood.down_timer_len - 1);
                    t.var.Wood.branch_count = me.var.Wood.branch_count;
                    grid.set(v.x + t.var.Wood.Dir.x, v.y + t.var.Wood.Dir.y, t);

                    bool growingSideways = me.var.Wood.Dir.x != 0;
                    //checks for growing branches sideways
                    if(me.var.Wood.branch_count < 5 && me.var.Wood.down_timer_len < TREE_MAX_BRANCH_SIZE - TREE_MIN_BRANCH_SIZE && g_rng.Random() < TREE_BRANCH_GROW_CHANCE) {
                        {
                            auto t = me;
                            t.var.Wood.branch_count += 1;
                            grid.set(v, t);
                        }

                        CellVar t(eCellType::Wood);
                        t.var.Wood.down_timer_len = g_rng.Random(TREE_MIN_BRANCH_SIZE, TREE_MAX_BRANCH_SIZE);
                        t.var.Wood.branch_count = me.var.Wood.branch_count;
                        if(growingSideways) {
                            t.var.Wood.Dir.y = 1;
                            t.var.Wood.Dir.x = 0;
                        } else {
                            t.var.Wood.Dir.y = 0;
                            t.var.Wood.Dir.x = g_rng.Random() > 0.5f ? -1 : 1;
                        }
                        grid.set(v, t);
                    //if not growing branches then reach equilibrium and set down_timer_len to stable 0
                    } else {
                        t.var.Wood.down_timer_len = 0;
                        grid.set(v, t);
                    }
                }
            // if reached end of branch and has been marked for growing leaves
            } else if(me.var.Wood.down_timer_len == -1) {
                auto t = CellVar(eCellType::Leaf);
                t.var.Leaf.down_timer_len = TREETOP_RADIUS;
                grid.set(v, t);
            }

        },
        //colors
        {clr_t(90, 40, 10)}
    );
#define LEAF_DUPE_CHANCE 0.5f
    CellVar::properties[eCellType::Leaf] = CellConstants(
        //powdery
        eState::Soild,
        //density
        256,
        //flammability
        0.1f,
        //behaviour
        [](vec2i v, Grid& grid) {
            auto& me = grid.get(v);
            const auto is_replaceable = [&](const int& otherx, const int& othery) {
                return grid.get(otherx, othery).type == eCellType::Air || grid.get(otherx, othery).type == eCellType::Wood;
            };
            const auto check_and_replace = [&](const int& x, const int& y, const CellVar& t) {
                if(is_replaceable(x, y) && g_rng.Random() < LEAF_DUPE_CHANCE) {
                    grid.set(x, y, t);
                }
            };
            if(me.var.Leaf.down_timer_len > 0) {
                auto t = me;
                t.var.Leaf.down_timer_len -= 1;
                check_and_replace(v.x, v.y + 1, t);
                check_and_replace(v.x, v.y - 1, t);
                check_and_replace(v.x + 1, v.y, t);
                check_and_replace(v.x - 1, v.y, t);
                grid.set(v, t);
            }

        },
        //colors
        {clr_t(50, 150, 50), clr_t(50, 170, 50), clr_t(50, 200, 50)}
    );
//determines flame size
#define MAX_FIRE_LIFETIME 30 
#define MIN_FIRE_LIFETIME 10 
    
#define SMOKE_CHANCE 0.1f

#define FIRE_COLOR_BLEND_AREA_HEIGHT 5
    CellVar::properties[eCellType::Fire] = CellConstants(
        eState::Gas,
        //density
        -10,
        //flammability
        0.f,
        //behaviour
        [](vec2i v, Grid& grid) {
            auto me = grid.get(v);
            static const std::vector<clr_t> mid_colors = { clr_t(226, 88, 34), clr_t(128, 9, 9) };
            const auto check_and_spread = [&](const int& otherx, const int& othery) {
                float flammability = grid.get(otherx, othery).getProperty().flammability;
                if(flammability != 0.f) {
                    float dice_roll = g_rng.Random();
                    //if rolled for fire spread
                    if(dice_roll < flammability) {
                        CellVar t(eCellType::Fire);
                        t.var.Gas.Fire.isSource = true;
                        grid.set(otherx, othery, t);
                        return true;
                    //if almost got spread then just chill on the surface
                    } else if(dice_roll < flammability * 2) {
                        CellVar t(eCellType::Fire);
                        t.var.Gas.Fire.isSource = true;
                        grid.set(v, t);
                        return true;
                    }
                }
                return false;
            };
            //spreading fire 
            {
                do {
                    if(check_and_spread(v.x, v.y + 1))
                        break;
                    if(check_and_spread(v.x, v.y - 1))
                        break;
                    if(check_and_spread(v.x + 1, v.y))
                        break;
                    if(check_and_spread(v.x - 1, v.y))
                        break;
                } while(false);
            }
            //if fire is source
            if(me.var.Gas.Fire.isSource == true) {
                //if no flammable material around you
                if(grid.get(v.x, v.y - 1).getProperty().flammability <= 0.f && grid.get(v.x, v.y + 1).getProperty().flammability <= 0.f &&
                    grid.get(v.x - 1, v.y).getProperty().flammability <= 0.f && grid.get(v.x + 1, v.y).getProperty().flammability <= 0.f)
                {
                    grid.set(v, eCellType::Air);
                    return;
                }
                if(grid.get(v.x, v.y + 1).type == eCellType::Air)
                    grid.set(v.x, v.y + 1, eCellType::Fire);
                if(grid.get(v.x, v.y - 1).type == eCellType::Air)
                    grid.set(v.x, v.y - 1, eCellType::Fire);
                if(grid.get(v.x-1, v.y).type == eCellType::Air)
                    grid.set(v.x-1, v.y, eCellType::Fire);
                if(grid.get(v.x+1, v.y).type == eCellType::Air)
                    grid.set(v.x+1, v.y, eCellType::Fire);
            //if fire is just normal floaty fire
            }else {
                //slow deprication
                if(me.age >= MIN_FIRE_LIFETIME) {
                    //the older the fire particle the more probable it is to annihilate
                    if(g_rng.Random() < Remap<int, float>(MIN_FIRE_LIFETIME, MAX_FIRE_LIFETIME, 0.f, 1.f).Convert(me.age)){
                        grid.set(v, g_rng.Random() < SMOKE_CHANCE ? eCellType::Smoke : eCellType::Air);
                        return;
                    }
                }
                //changing colors
                clr_t cur = me.color;
                for(int i = 1; i < mid_colors.size() + 1; i++) {
                    //create 'sectors' for each color of the flame with FIRE_COLOR_BLEND_AREA_HEIGHT as max blend height between colors
                    if(me.age + g_rng.Random<int>(0, FIRE_COLOR_BLEND_AREA_HEIGHT) >= MIN_FIRE_LIFETIME / mid_colors.size() * i) {
                        me.color = mid_colors[i - 1];
                        continue;
                    } 
                    break;
                }
                if(cur != me.color) {
                    grid.set(v, me);
                }
                //movement behaviour
                CellVar::properties[eCellType::Smoke].update_behaviour(v, grid);
            }
        },
        //color will be changing through lifetime
        {clr_t(253,207,88)}
    );

    CellVar::properties[eCellType::Bedrock] = CellConstants(
        //powdery
        eState::Soild,
        //density
        0xffffff,
        //flammability
        0.f,
        //behaviour
        [](vec2i v, Grid& grid) {
            return;
        },
        //colors
        {clr_t(50, 50, 50)}
    );
    //adding reaction with acid to everything
    for(auto& p : CellVar::properties) {
        if(p.first != eCellType::Acid && p.first != eCellType::Smoke && p.first != eCellType::Air && p.first != eCellType::Bedrock) {
            p.second.reactions[eCellType::Acid] = { eCellType::Air, ACID_REACTION_PROBABILITY };
            CellVar::properties[eCellType::Acid].reactions[p.first] = { eCellType::Smoke, ACID_REACTION_PROBABILITY };
        }
    }
#define bSHOULD_GAS_ANNIGILATE false
#define GAS_ANNIHILIATION_CHANCE 0.001f

#if bSHOULD_GAS_ANNIGILATE
    for(auto& p : CellVar::properties) {
        if(p.second.state == eState::Gas)
            p.second.reactions[eCellType::Air] = {eCellType::Air, GAS_ANNIHILIATION_CHANCE};
    }
#endif
}
