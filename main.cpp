#include <iostream>

#include <SFML/Graphics.hpp>

#include "imgui.h"
#include "imgui-SFML.h"

#include "coroutines.hpp"
#include "utils.h"
#include "core.h"
#include "timer.h"
#include "sprite.h"
#include "shape.h"
#include "rigidbody.h"
#include "quarry.h"
#include "entity.h"

#define GWH 512
#define GWW 512

#define WH 2048.f
#define WW (WH * GWH/GWW)


const static QuarrySprite g_player_sprite("./assets/player.png");
const static QuarrySprite g_camera_sprite("./assets/camera.png");


class Demo : public QuarryApp {
public:
    enum eEntityType {
        PlayerT,
        GenPickupT,
        CameraPickupT,
    };
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
            auto held_obj_info = this->GetVar<void*>("obj_held");
            Entity* held_obj = nullptr;
            if(held_obj_info.has_value() && held_obj_info.value() != nullptr)
                held_obj = (Entity*)held_obj_info.value();

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
            if(sf::Mouse::isButtonPressed(sf::Mouse::Right) && held_obj) {
                held_obj->rb.isActive = true;
                held_obj->rb.vel = this->rb.vel * 2.f;
                if(held_obj->rb.vel.y < 0) {
                    held_obj->rb.vel.y = 1.f;
                }
                //to avoid instant pickup
                held_obj->pos += held_obj->rb.vel;

                held_obj->SetVar("parent", nullptr);
                SetVar("obj_held", nullptr);
            }
            auto cursor_pos_info = GetVar<vec2f>("cursor_pos");
            //making photos
            if(cursor_pos_info.has_value() && held_obj && held_obj->type == eEntityType::CameraPickupT) {
                vec2f mpos = cursor_pos_info.value();
                Shape shp;

                float fram_size = held_obj->GetVar<float>("frame_size").has_value() ? held_obj->GetVar<float>("frame_size").value() : 0.f;
                float range = held_obj->GetVar<float>("range").has_value() ? held_obj->GetVar<float>("range").value() : 0.f;

                vec2f clamped_mpos = normal(mpos - this->pos) * std::clamp(length(mpos - this->pos), length(mpos - this->pos), range);

                auto frame = AABBCS<float>(clamped_mpos + pos, vec2f(fram_size, fram_size));
                frame.min.x = std::max<float>(0, frame.min.x);
                frame.min.y = std::max<float>(0, frame.min.y);
                frame.max.x = std::min<float>(frame.max.x, g.getWidth() - 1);
                frame.max.y = std::min<float>(frame.max.y, g.getHeight() - 1);
                auto frame_ext = AABBCS<float>(frame.center(), frame.size() + vec2f(0.01f, 0.01f));

                vec2f camera_origin = this->pos + vec2f(0, this->rb.radius + held_obj->rb.radius);
                std::vector<vec2f> rays = {frame_ext.min, frame_ext.max, vec2f(frame_ext.min.x, frame_ext.max.y), vec2f(frame_ext.max.x, frame_ext.min.y)};
                for(auto& r : rays) {
                    float t;
                    if(!RayVAABB(camera_origin, r - camera_origin, frame)) {
                        shp.add(RayAB(camera_origin, r));
                    }
                }
                shp.add(frame);
                clr_t frame_clr = clr_t(127, 127, 127);
                auto framei = AABBi((vec2i)frame.min, (vec2i)frame.max);

                static Map picture;
                static QuarrySprite frame_sprite(1, 1, clr_t::White);
                frame_sprite.flip.x = 1;
                static epi::Coroutine* coro = new epi::Coroutine;
                auto handle_picture_making = [&] () {
                    EPI_CoroutineBegin(coro);
                    EPI_CoroutineYieldUntil(coro, sf::Mouse::isButtonPressed(sf::Mouse::Left));

                    {
                        picture = g.exportToMap(framei);
                        frame_clr = clr_t::White;
                        std::vector<clr_t> clrs;
                        for(auto& c : picture.data)
                            clrs.push_back(clr_t(c.color.r, c.color.g, c.color.b, 64));
                        frame_sprite = QuarrySprite(picture.w, picture.h, clrs);
                    }

                    EPI_CoroutineWait(coro, 0.25);
                    EPI_CoroutineYieldUntil(coro, sf::Mouse::isButtonPressed(sf::Mouse::Left));

                    g.importFromMap(framei, picture);
                    g.redrawSegment(framei);
                    g.updateSegment(framei.min - vec2i(1, 1), framei.max + vec2i(1, 1));
                    picture.data.clear();
                    frame_sprite = QuarrySprite(picture.w, picture.h);

                    EPI_CoroutineWait(coro, 0.25);
                    EPI_CoroutineReset(coro);
                };
                handle_picture_making();
                frame_sprite.drawAt((vec2i)frame.center(), g);
                shp.draw(g, frame_clr);
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
            type = eEntityType::PlayerT;
            SetVar("obj_held", (void*)nullptr);
        }
    };
    class PickupObject : public Entity {
    public:
        void onHitEntity(Entity* e) override {
            if(e->type == eEntityType::PlayerT) {
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
            this->type = eEntityType::GenPickupT;
            SetVar("parent", nullptr);
            this->rb.physics.drag *= 0.3f;
        }
    };
    class CameraPickup : public PickupObject {
        public:
        CameraPickup(vec2f p, const QuarrySprite& spr) : PickupObject(p, spr) {
            this->type = eEntityType::CameraPickupT;
            SetVar("range", 150.f);
            SetVar("frame_size", 50.f);
        }
    };
    std::vector<std::unique_ptr<Entity> > entities;
    Player* player;
    CameraPickup* camera;

    int brush_size;
    bool on_brush_click = false;
    bool editor_mode = true;
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

        if(editor_mode) {
            if (sf::Mouse::isButtonPressed(sf::Mouse::Left) && !on_brush_click) {
                spawnAtBrush(grid);
            }
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
            shp.add(new_view);
            shp.draw(grid, clr_t::Cyan);
        }

        player->SetVar("cursor_pos", (vec2f)grid_mouse_coords);
        for(auto& e : entities) {
            if(!editor_mode) {
                e->rb.vel.y -= 0.07f;
                e->update(grid);
                e->rb.update(grid, e->pos);
                for(auto& ee : entities) {
                    if(e != ee)
                        e->processEntityCollisions(ee.get());
                }
            }
            e->draw(grid);
        }
        //p_frag_shader.setUniform("u_src_pos", (player->pos - (vec2f)grid.getViewWindow().min) * ((float)WW / grid.getViewWindow().size().x) );

        {
            ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(ImGuiWindowSize.x, ImGuiWindowSize.y), ImGuiCond_FirstUseEver);

            ImGui::Begin("Demo window", nullptr);

            //printing material under cursor
            ImGui::Text("%s", to_str(grid.get(grid_mouse_coords).type) + sizeof("eCellType:"));

            //printing fps
            ImGui::Text("%f", p_avg_fps);
            //show updated segments
            if(ImGui::Button("edit")) {
                editor_mode = !editor_mode;
            }
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
        //bindFragShader("assets/light_shader.frag");
        //p_frag_shader.setUniform("u_src_range", 500.f* ((float)WW / GWW));
        //p_frag_shader.setUniform("u_src_intensity", 1.5f);
        brush_size = std::clamp<int>(sqrt(grid.getViewWindow().size().x * grid.getViewWindow().size().y) / 32 / 2, 1, 0xffffff);
        CellVar::addReplicatorMap("tree.bin", 0);
        CellVar::addReplicatorMap("house.bin", 1);
        CellVar::addReplicatorMap("horse.bin", 2);
        CellVar::addReplicatorMap("island.bin", 3);

        player = new Player(vec2f(50, 50));
        entities.push_back(std::unique_ptr<Entity>(player));
        camera = new CameraPickup(vec2f(100, 50), g_camera_sprite);
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
                if(!editor_mode)
                    return;
                auto t = brush_size;
                brush_size = 1;
                spawnMaterial(grid, sf::Keyboard::Q, eCellType::Seed);
                brush_size = t;
            });
        addKeyHook(sf::Keyboard::S, 
            [&]() {
                if(editor_mode)
                    return;
                vec2i mouse_pos = getMousePos();
                auto grid_mouse_coords = grid.convert_coords(mouse_pos, p_window_size); 
                camera = new CameraPickup((vec2f)grid_mouse_coords, g_camera_sprite);
                entities.push_back(std::unique_ptr<Entity>(camera));
            });
        addKeyHook(sf::Keyboard::Num0, 
            [&]() {
                if(!editor_mode)
                    return;
                SPAWN_REPLICATOR(0, 0, 0);
            });
        addKeyHook(sf::Keyboard::Num1, 
            [&]() {
                if(!editor_mode)
                    return;
                SPAWN_REPLICATOR(1, 0, 0);
            });
        addKeyHook(sf::Keyboard::Num2, 
            [&]() {
                if(!editor_mode)
                    return;
                SPAWN_REPLICATOR(2, 0, 0);
            });
        addKeyHook(sf::Keyboard::Num3, 
            [&]() {
                if(!editor_mode)
                    return;
                SPAWN_REPLICATOR(3, 0, 0);
            });
        addKeyHook(sf::Keyboard::W, 
            [&]() {
                if(editor_mode)
                    return;
                if(player->isGrounded)
                    player->rb.vel.y = 3.f;
                if(player->isSemiGrounded)
                    player->rb.vel.y += 1.f;
            });
        addKeyHook(sf::Keyboard::R, 
            [&]() {
                if(!editor_mode)
                    return;
                grid = Grid(GWW, GWH);
            });
        addKeyHook(sf::Keyboard::V, 
            [&]() {
                grid.setViewWindow(grid.getDefaultViewWindow());
            });
        addKeyHook(sf::Keyboard::Space, 
            [&]() {
                editor_mode = !editor_mode;
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
