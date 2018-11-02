#pragma once
#include "hex.hpp"
#include "minomaly/minomaly.hpp"
#include "minomaly/system/input_system.hpp"
#include "minomaly/system/render_system.hpp"
#include "quadtree.hpp"
#include <iostream>
#include <random>

class MapSystem final : public mino::ISystem
{
    mino::Minomaly& engine;
    mino::RenderSystem* render_system = nullptr;
    mino::InputSystem* input_system = nullptr;
    mino::Manager<HexComponent>* hex_component_manager = nullptr;
    mino::Manager<mino::RenderComponent>* render_component_manager = nullptr;
    mino::Logger* logger = nullptr;

    Quadtree<bool> cells{mino::AABB{MAP_SIZE_X / 2, MAP_SIZE_Y / 2, MAP_SIZE_X, MAP_SIZE_Y / 2}};

    uint8_t map_iterations = 0;

public:
    explicit MapSystem(mino::Minomaly& engine);
    ~MapSystem() = default;

    void start() override;
    void update() override;

private:
    void init_cells();
    void update_cells();
    void generate_labyrinth();
};

