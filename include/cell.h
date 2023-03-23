#pragma once
#include "utils.h"
#include <map>
#include <initializer_list>
#include <functional>

namespace epi {
class Grid;
class Map;

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
    Rock,
    Dirt,
    CompressedDirt,
    Crystal,

    CrumblingStone,
    //solids
    Stone,
    //special
    //tree & vegetation
    Replicator,
    Grass,
    Seed,
    Wood,
    Leaf,

    Fire,

    Bedrock,
    COUNT,
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
    std::vector<Color> colors;

    CellConstants() {}
    CellConstants(eState state_of_matter, int density_, float flammability_, std::function<void(vec2i, Grid&)> update_func_, std::initializer_list<Color> colors_) 
        : state(state_of_matter), density(density_), flammability(flammability_), update_behaviour( update_func_), colors(colors_) {}
};

struct CellVar {
private:
    unsigned int m_getNextID() {
        static unsigned int next_id = 1;
        return next_id++;
    }
    unsigned int id;
    unsigned int last_tick_updated;


    //maps can be of max size 
    static std::map<unsigned char, Map> replicator_maps;
public:
    static CellConstants properties[(size_t)eCellType::COUNT];
    unsigned long getID() const {return id;}

    //if not working means you havent called InitializeProperties()

    eCellType type;
    Color color;
    //number of updates
    unsigned short move_count = 0;
    float temp;
    unsigned short age = 0;

    static void addReplicatorMap(const char* filename, unsigned char id);
    union VarUnion {
        //max size of short, filename to bin file and id to override/add new map
        struct {
            //cooridnates in respect to the center
            short x;
            short y;
            //id of map trying to replicate
            unsigned char id;
        }Replicator;
        struct {
            bool isSource;
        }Fire;
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
            bool isEdge;
        }Sand;
        struct {
            char isSupported;
        }Rock;
        struct {
            unsigned char grass_timer;
            bool bgrew_grass;
        }Dirt;
        struct {
            char current_side;
            char state;
            char parent_id;
        }CompressedDirt;
        struct {
            unsigned char lvl;
            bool hasCrumbled;
        }CrumblingStone;
        struct {
            unsigned char down_timer_len;
            unsigned char my_dirt_id;
        }Grass;
        struct {
            unsigned char down_timer_len;
        }Leaf;
        struct {
            char CurSide;
        }Crystal;
    }var;

    const CellConstants& getProperty() const {
        return properties[static_cast<size_t>(type)];
    }
    CellVar(eCellType type_) 
        : type(type_), id(m_getNextID()), age(0), move_count(0), last_tick_updated(0)
    {
        std::memset(&var, 0, sizeof(VarUnion));

        auto& all_colors = properties[static_cast<size_t>(type)].colors;
        color = all_colors[ g_rng.Random<size_t>(0U, all_colors.size()) ];
    }
    friend Grid;
    friend VarUnion;
    static void InitializeProperties();
};
}
