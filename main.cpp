#include <iostream>

#include <SFML/Graphics.hpp>

#include "imgui.h"
#include "imgui-SFML.h"

#include "utils.h"
#include "core.h"
#include "timer.h"
#include "sprite.h"
#include "render_shape.h"
#include "rigidbody.h"
#include "quarry.h"
#include "entity.h"

#define GWH 512
#define GWW 512

#define WH 2048.f
#define WW (WH * GWH/GWW)


const static QuarrySprite g_player_sprite("./assets/player.png");
const static QuarrySprite g_camera_sprite("./assets/camera.png");

#define ENTITY_PLAYER_TYPE 69

class Player : public Entity {
public:
    //in degrees
    float grounded_check_angle = 35.f;
    size_t grounded_check_count = 5;

    bool isGrounded = true;
    bool isSemiGrounded = true;

    void onHitEntity(Entity* e) override {
        isSemiGrounded = true;
    }
    void update(Grid& g) override {
        vec2i player_input = {0, 0};
        float player_speed = 0.05f;
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::W))
            player_input.y = 1;
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::S))
            player_input.y = -1;
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::A))
            player_input.x = -1;
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::D))
            player_input.x = 1;
        if(player_input.x != 0 || player_input.y != 0)
            rb.vel.x += (float)player_input.x * player_speed;

        //for throwing
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::H)) {
            auto o = GetVar<void*>("obj_held");
            if(o.has_value() && o.value() != nullptr) {
                Entity* e = (Entity*)o.value();
                e->rb.isActive = true;
                e->rb.vel = this->rb.vel * 2.f;
                if(e->rb.vel.y < 0) {
                    e->rb.vel.y = 1.f;

                }
                //to avoid instant pickup
                e->pos += e->rb.vel;

                e->SetVar("parent", nullptr);
                SetVar("obj_held", nullptr);
            }
        }

        isGrounded = false;
        isSemiGrounded = false;
        for(float a = -grounded_check_angle / 2.f; a < grounded_check_angle; a += grounded_check_angle / grounded_check_count) {
            float rad = a / 360.f * 2.f * 3.141f;
            auto cur_check = g.get(pos.x + sin(rad) * rb.radius * 1.5f, pos.y - cos(rad) * rb.radius * 1.5f);
            if(cur_check.getProperty().state == eState::Powder || cur_check.getProperty().state == eState::Soild)
                isGrounded = true;
        }
        if(g.get((vec2i)pos).getProperty().state == eState::Liquid)
            isSemiGrounded = true;
    }

    Player(vec2f p) : Entity(p, g_player_sprite) {
        rb.physics.density = 900;
        rb.physics.friction *= 0.5f;
        type = ENTITY_PLAYER_TYPE;
        SetVar("obj_held", (void*)nullptr);
    }
};
#define ENTITY_PICKUP_TYPE 1
class PickupObject : public Entity {
public:
    void onHitEntity(Entity* e) override {
        if(e->type == ENTITY_PLAYER_TYPE) {
            auto v = e->GetVar<void*>("obj_held");
            if(v.value() == nullptr) {
                e->SetVar("obj_held", this);
                this->rb.isActive = false;
                SetVar("parent", e);
            }
        }
    }
    void update(Grid& g) override {
        Entity* e = (Entity*)GetVar<void*>("parent").value();
        if(e) {
            this->pos = e->pos + vec2f(0, e->rb.radius * 1 + this->rb.radius);
        }
    }

    PickupObject(vec2f p, const QuarrySprite& spr) : Entity(p, spr) {
        this->type = ENTITY_PICKUP_TYPE;
        SetVar("parent", nullptr);
        this->rb.physics.drag *= 0.3f;
    }
};

class Demo : public QuarryApp {
public:
    std::vector<std::unique_ptr<Entity> > entities;
    Player* player;
    PickupObject* camera;

    int brush_size;
    bool on_brush_click = false;
    eCellType brush_material = eCellType::Sand;
    vec2i last_mouse_pos = {-1, -1};
    vec2i grid_mouse_coords;

    vec2f ImGuiWindowSize = vec2f(WW/5, WH / 2);

    vec2i player_input = {0, 0};

    void spawnAtBrush(Grid& grid, bool ifEmpty = false, const CellVar* cv = nullptr ) {
        vec2i mouse_pos = getMousePos();
        auto grid_mouse_coords = grid.convert_coords(mouse_pos, p_window_size); 
        for(int y = -brush_size/2; y < round((float)brush_size/2.f); y++)
            for(int x = -brush_size/2; x < round((float)brush_size/2.f); x++) {
                if(grid.inBounds(grid_mouse_coords + vec2i(x, y)) && (grid.get(grid_mouse_coords + vec2i(x, y)).type == eCellType::Air || !ifEmpty)){
                    if(cv) 
                        grid.set(grid_mouse_coords + vec2i(x, y), *cv);
                    else
                        grid.set(grid_mouse_coords + vec2i(x, y), CellVar(brush_material));
                }
            }
    }
    void spawnMaterial(Grid& grid, sf::Keyboard::Key k, eCellType type) {
        brush_material = type;
        if(sf::Keyboard::isKeyPressed(k)){
            spawnAtBrush(grid);
        }
    };
    void update(Grid& grid) override {

        if (sf::Mouse::isButtonPressed(sf::Mouse::Left) && !on_brush_click) {
            spawnAtBrush(grid);
        }
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::LShift)) {
            spawnMaterial(grid, sf::Keyboard::A, eCellType::Acid);
            spawnMaterial(grid, sf::Keyboard::E, eCellType::Leaf);
            spawnMaterial(grid, sf::Keyboard::O, eCellType::Wood);
            spawnMaterial(grid, sf::Keyboard::D, eCellType::Dirt);
            spawnMaterial(grid, sf::Keyboard::S, eCellType::Sand);
            spawnMaterial(grid, sf::Keyboard::W, eCellType::Water);
            spawnMaterial(grid, sf::Keyboard::L, eCellType::Lava);
            spawnMaterial(grid, sf::Keyboard::X, eCellType::Stone);
            spawnMaterial(grid, sf::Keyboard::C, eCellType::Crystal);
            spawnMaterial(grid, sf::Keyboard::F, eCellType::Fire);
        }


        vec2i mouse_pos = getMousePos();
        grid_mouse_coords = grid.convert_coords(mouse_pos, p_window_size); 
        if( sf::Keyboard::isKeyPressed(sf::Keyboard::Z)) {
            AABBf new_view;

            vec2i this_mouse_pos = getMousePos();
            this_mouse_pos= grid.convert_coords(this_mouse_pos, p_window_size);

            new_view.min.x = std::min(this_mouse_pos.x, last_mouse_pos.x);
            new_view.min.y = std::min(this_mouse_pos.y, last_mouse_pos.y);

            new_view.max.x = std::max(this_mouse_pos.x, last_mouse_pos.x);
            new_view.max.y = std::max(this_mouse_pos.y, last_mouse_pos.y);

            int max_size = std::max(new_view.size().x, new_view.size().y);

            new_view.max.x = new_view.min.x + max_size;
            new_view.max.y = new_view.min.y + max_size;

            new_view.min -= vec2f(1, 1);
            Shape shp;
            shp.add(new_view, clr_t::Cyan);
            shp.draw(grid);
        }

        for(auto& e : entities) {
            e->rb.vel.y -= 0.07f;
            e->update(grid);
            e->rb.update(grid, e->pos);
            e->draw(grid);
            for(auto& ee : entities) {
                if(e != ee)
                    e->processEntityCollisions(ee.get());
            }
        }
        {
            ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(ImGuiWindowSize.x, ImGuiWindowSize.y), ImGuiCond_FirstUseEver);

            ImGui::Begin("Demo window", nullptr);

            //printing material under cursor
            ImGui::Text("%s", to_str(grid.get(grid_mouse_coords).type) + sizeof("eCellType:"));

            //printing fps
            ImGui::Text("%f", p_avg_fps);
            //show updated segments
            if(ImGui::Button("show change")) {
                grid.debug.isActive = !grid.debug.isActive;
            }
            if(grid.debug.isActive) {
                if(ImGui::Button("show updates")) {
                    grid.debug.showUpdated = !grid.debug.showUpdated;
                }
                if(grid.debug.showUpdated) {
                    static float col[3] = {grid.debug.update_clr.r / 255.f, grid.debug.update_clr.g / 255.f, grid.debug.update_clr.b / 255.f};
                    ImGui::ColorEdit3("change update color", col); 
                    grid.debug.update_clr.r = col[0] * 255;
                    grid.debug.update_clr.g = col[1] * 255;
                    grid.debug.update_clr.b = col[2] * 255;
                }
                if(ImGui::Button("show draws")) {
                    grid.debug.showDraws = !grid.debug.showDraws;
                }
                if(grid.debug.showDraws) {
                    static float col[3] = {grid.debug.draw_clr.r / 255.f, grid.debug.draw_clr.g / 255.f, grid.debug.draw_clr.b / 255.f};
                    ImGui::ColorEdit3("change draw color", col); 
                    grid.debug.draw_clr.r = col[0] * 255;
                    grid.debug.draw_clr.g = col[1] * 255;
                    grid.debug.draw_clr.b = col[2] * 255;
                }
            }
            static std::string filename = "test.bin";
            ImGui::Text("filename: \"%s\"", filename.c_str());
            if(ImGui::Button("export")) {
                std::cin >> filename;
                grid.exportToFile(filename.c_str());
            }
            if(ImGui::Button("import")) {
                std::cin >> filename;
                auto tmp_grid = Grid(1, 1);
                tmp_grid.importFromFile(filename.c_str());
                grid.mergeAt(tmp_grid);
            }
            //brush size
            ImGui::SliderInt("brush", &brush_size, 1, 32);
            //material selection
            {
                std::vector<eCellType> all_materials;
                for(auto i = eCellType::Air; i <= eCellType::Bedrock; i = (eCellType)((int)i + 1))
                    all_materials.push_back(i);
                std::vector<const char*> all_materials_str;
                for(auto i : all_materials) {
                    //ohh so haacky
                    all_materials_str.push_back(to_str(i) + sizeof("eCellType:"));
                }

                static int item_current = 0;
                ImGui::ListBox("mat", &item_current, &all_materials_str[0], all_materials_str.size(), 17);
                brush_material = all_materials[item_current];

                on_brush_click = false;
                if(brush_material == eCellType::Seed){
                    on_brush_click = true;
                    brush_size = 1;
                }
            }
            ImGui::End();
        }
    }
    bool setup(Grid& grid) override {
        brush_size = std::clamp<int>(sqrt(grid.getViewWindow().size().x * grid.getViewWindow().size().y) / 32 / 2, 1, 0xffffff);
        CellVar::addReplicatorMap("tree.bin", 0);
        CellVar::addReplicatorMap("house.bin", 1);
        CellVar::addReplicatorMap("horse.bin", 2);
        CellVar::addReplicatorMap("island.bin", 3);

        player = new Player(vec2f(50, 50));
        entities.push_back(std::unique_ptr<Entity>(player));
        camera = new PickupObject(vec2f(100, 50), g_camera_sprite);
        entities.push_back(std::unique_ptr<Entity>(camera));

#define SPAWN_REPLICATOR(cid, cx, cy)\
            auto t = brush_size;\
\
            CellVar cv(eCellType::Replicator);\
            cv.var.Replicator.id = cid;\
            cv.var.Replicator.x= cx;\
            cv.var.Replicator.y= cy;\
\
            brush_size = 1;\
            spawnAtBrush(grid, false, &cv);\
\
            brush_size = t

        addKeyHook(sf::Keyboard::Q, 
            [&]() {
                auto t = brush_size;
                brush_size = 1;
                spawnMaterial(grid, sf::Keyboard::Q, eCellType::Seed);
                brush_size = t;
            });
        addKeyHook(sf::Keyboard::Num0, 
            [&]() {
                SPAWN_REPLICATOR(0, 0, 0);
            });
        addKeyHook(sf::Keyboard::Num1, 
            [&]() {
                SPAWN_REPLICATOR(1, 0, 0);
            });
        addKeyHook(sf::Keyboard::Num2, 
            [&]() {
                SPAWN_REPLICATOR(2, 0, 0);
            });
        addKeyHook(sf::Keyboard::Num3, 
            [&]() {
                SPAWN_REPLICATOR(3, 0, 0);
            });
        addKeyHook(sf::Keyboard::W, 
            [&]() {
                if(player->isGrounded)
                    player->rb.vel.y = 3.f;
                if(player->isSemiGrounded)
                    player->rb.vel.y += 1.f;
            });
        addKeyHook(sf::Keyboard::R, 
            [&]() {
                grid = Grid(GWW, GWH);
            });
        addKeyHook(sf::Keyboard::V, 
            [&]() {
                grid.setViewWindow(grid.getDefaultViewWindow());
            });
        addKeyHook(sf::Keyboard::Space, 
            [&]() {
                p_isUpdating = !p_isUpdating;
            });
        addKeyHook(sf::Keyboard::Z, 
            [&]() {
                if(last_mouse_pos.x == -1) {
                    last_mouse_pos = getMousePos();
                    last_mouse_pos = grid.convert_coords(last_mouse_pos, p_window_size);
                }
            }, eKeyHookState::isPressed);
        addKeyHook(sf::Keyboard::Z, 
            [&]() {
                AABBi new_view;

                vec2i this_mouse_pos = getMousePos();
                this_mouse_pos= grid.convert_coords(this_mouse_pos, p_window_size);

                new_view.min.x = std::min(this_mouse_pos.x, last_mouse_pos.x);
                new_view.min.y = std::min(this_mouse_pos.y, last_mouse_pos.y);
                new_view.max.x = std::max(this_mouse_pos.x, last_mouse_pos.x);
                new_view.max.y = std::max(this_mouse_pos.y, last_mouse_pos.y);
                int max_size = std::max(new_view.size().x, new_view.size().y);
                new_view.max.x = new_view.min.x + max_size;
                new_view.max.y = new_view.min.y + max_size;

                new_view.min -= vec2i(1, 1);

                grid.setViewWindow(new_view);
                last_mouse_pos.x = -1;
            }, eKeyHookState::isReleased);

        return true;
    }

    Demo() : QuarryApp(vec2i(GWW, GWH), vec2f(WW, WH)) {}
};
int main()
{
    Demo app;
    app.run();
    return 0;
}
