#pragma once
#include "utils.h"
#include <map>
#include <initializer_list>
#include <functional>

class Grid;

enum class eCellType : unsigned char {
    //gases
    Air = 0,
    Smoke,
    Steam,
    //liquids
    Water,
    Lava,
    Acid,
    //solids [powders]
    Sand,
    Cobblestone,
    Dirt,
    Crystal,
    //solids
    Stone,
    //special
    //tree & vegetation
    Grass,
    Seed,
    Wood,
    Leaf,

    Fire,

    Bedrock,
};
const char* to_str(eCellType type);
enum eState {
    Gas,
    Liquid,
    Soild,
    Powder,
};
struct CellReaction {
    eCellType product;
    float probability = 1.f;

    CellReaction() : product(eCellType::Air), probability(1.f) {}
    CellReaction(eCellType result_, float probability_ = 1.f) : product(result_), probability(probability_) {}
};
struct CellConstants {
    eState state;
    int density;
    float flammability;
    std::function<void(vec2i, Grid&)> update_behaviour;
    std::vector<clr_t> colors;

    CellConstants() {}
    CellConstants(eState state_of_matter, int density_, float flammability_, std::function<void(vec2i, Grid&)> update_func_, std::initializer_list<clr_t> colors_) 
        : state(state_of_matter), density(density_), flammability(flammability_), update_behaviour( update_func_), colors(colors_) {}
};

void InitializeProperties();

struct CellVar {
private:
    unsigned int m_getNextID() {
        static unsigned int next_id = 1;
        return next_id++;
    }
    unsigned int id;
    unsigned int last_tick_updated;

    unsigned long getID() const {return id;}
public:
    //if not working means you havent called InitializeProperties()
    static std::map<eCellType, CellConstants> properties;

    eCellType type;
    clr_t color;
    //number of updates
    unsigned short age;

    union VarUnion {
        struct {
            unsigned short move_count;
            struct {
                bool isSource;
            }Fire;

        }Unstable;
        //everything that uses sand update behaviour
        struct {
            unsigned char branch_count;
            char down_timer_len;
            struct Dir_t {
                char x;
                char y;
                void set(char x_, char y_) { x = x_; y = y_; }
                void set(vec2i v) { x = v.x; y = v.y; }
            }Dir;
        }Wood;
        struct {
            unsigned char grass_timer;
            bool bgrew_grass;
        }Dirt;
        struct {
            unsigned char down_timer_len;
        }Grass;
        struct {
            unsigned char down_timer_len;
        }Leaf;
        struct {
            char CurSide;
        }Crystal;
    }var;

    const CellConstants& getProperty() const {
        return properties[type];
    }
    CellVar(eCellType type_) 
        : type(type_), id(m_getNextID()), age(0), last_tick_updated(0)
    {
        std::memset(&var, 0, sizeof(VarUnion));

        auto& all_colors = properties[type].colors;
        color = all_colors[ g_rng.Random<size_t>(0U, all_colors.size()) ];
    }
    friend Grid;
};
