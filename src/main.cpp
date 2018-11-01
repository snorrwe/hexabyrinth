#define _SILENCE_CXX17_RESULT_OF_DEPRECATION_WARNING
#include "hex.hpp"
#include "minomaly/minomaly.hpp"
#include "minomaly/system/input_system.hpp"
#include "minomaly/system/render_system.hpp"
#include "quadtree.hpp"
#include <iostream>
#include <random>

using namespace mino;

class MapSystem final : public ISystem
{
    Minomaly& engine;
    RenderSystem* render_system = nullptr;
    InputSystem* input_system = nullptr;
    Manager<HexComponent>* hex_component_manager = nullptr;
    Manager<RenderComponent>* render_component_manager = nullptr;
    Logger* logger = nullptr;

    Quadtree<bool> cells{AABB{MAP_SIZE_X / 2, MAP_SIZE_Y / 2, MAP_SIZE_X, MAP_SIZE_Y / 2}};

    uint8_t map_iterations = 0;

public:
    explicit MapSystem(Minomaly& engine)
        : engine(engine)
        , hex_component_manager(engine.create_component_manager<HexComponent>())
        , logger(engine.get_log_manager()->get_logger("map_system"))
    {
        assert(hex_component_manager);
    }
    ~MapSystem() = default;

    void start() override
    {
        logger->info("MapSystem is starting");

        render_system = engine.get_system<RenderSystem>();
        input_system = engine.get_system<InputSystem>();
        assert(render_system);
        assert(input_system);

        render_component_manager = engine.get_manager<Manager<RenderComponent>>();
        assert(render_component_manager);

        logger->info("Initializing labyrinth");
        PositionComponent map_offset{75, 50};
        for (auto y = 0; y < MAP_SIZE_Y; ++y)
        {
            auto offset = -y / 2;
            for (auto x = 0; x < MAP_SIZE_X; ++x)
            {
                auto entity = engine.add_entity();
                auto& hex_component = hex_component_manager->add_component(entity.id);
                hex_component.q = offset + x;
                hex_component.r = y;
                cells.add(Point{{hex_component.q, hex_component.r}}, false);

                auto components = render_system->create_renderable_entity(entity);
                components.position = hex_component.as_position() + map_offset;
                components.render.texture = render_system->load_texture("data/wall.png");
                components.render.source = {0, 0, 256, 256};
                components.render.dest = {0, 0, HEX_TEXTURE_SIZE, HEX_TEXTURE_SIZE};
            }
        }
        generate_labyrinth();
        logger->info("MapSystem started successfully");
    }

    void update() override
    {
        auto const& events = input_system->get_events();
        for (auto& event : events)
        {
            if (event.type == SDL_KEYDOWN)
            {
                if (event.key.keysym.sym == SDLK_SPACE)
                {
                    generate_labyrinth();
                }
                else if (event.key.keysym.sym == SDLK_q)
                {
                    logger->info("Exiting");
                    engine.stop();
                }
            }
        }
        if (map_iterations != 0)
        {
            update_cells();
            --map_iterations;
        }
    }

private:
    void update_cells()
    {
        hex_component_manager->iter([&](auto const& entity, auto const& component) {
            const auto neighbours = std::array{
                component.neighbour(Direction::NE),
                component.neighbour(Direction::E),
                component.neighbour(Direction::SE),
                component.neighbour(Direction::SW),
                component.neighbour(Direction::W),
                component.neighbour(Direction::NW),
            };
            auto n_neighbours = 0;
            for (auto& neighbour : neighbours)
            {
                auto const p = Point{{neighbour.q, neighbour.r}};
                auto r = cells.get(p);
                if (r != nullptr && *r)
                {
                    ++n_neighbours;
                }
            }
            auto const p = Point{{component.q, component.r}};
            auto r = cells.get(p);
            if (r == nullptr)
            {
                return;
            }
            auto const is_alive = *r;
            auto render_component = render_component_manager->get_component(entity);
            assert(render_component);
            if (is_alive)
            {
                if (1 > n_neighbours || n_neighbours > 2)
                {
                    *r = false;
                    render_component->texture = nullptr;
                }
            }
            else if (n_neighbours == 2)
            {
                *r = true;
                render_component->texture = render_system->load_texture("data/wall.png");
            }
        });
    }

    void generate_labyrinth()
    {
        logger->info("Generating new labyrinth");

        std::mt19937 rng;
        rng.seed(std::random_device()());
        std::uniform_int_distribution<std::mt19937::result_type> dist(1, 255);
        hex_component_manager->iter([&](auto const& entity, auto const& component) {
            auto const point = Point{{component.q, component.r}};
            auto r = cells.get(point);
            if (r == nullptr)
            {
                return;
            }
            auto p = dist(rng);
            auto texture = render_system->load_texture("data/wall.png");
            if ((p & 1) == 0)
            {
                texture = nullptr;
            }
            *r = texture != nullptr;
            auto render_component = render_component_manager->get_component(entity);
            assert(render_component);
            render_component->texture = texture;
        });

        map_iterations = 10;

        logger->info("Labyrinth generation done");
    }
};

int main(int argc, char** argv)
{
    auto engine = Minomaly();
    engine.create_system<MapSystem>(engine);

    engine.run();
    return 0;
}

