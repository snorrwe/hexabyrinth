#include "map_system.hpp"

MapSystem::MapSystem(mino::Minomaly& engine)
    : engine(engine)
    , hex_component_manager(engine.create_component_manager<HexComponent>())
    , logger(engine.get_log_manager()->get_logger("map_system"))
{
}

void MapSystem::start()
{
    logger->info("MapSystem is starting");

    render_system = engine.get_system<mino::RenderSystem>();
    assert(render_system);
    input_system = engine.get_system<mino::InputSystem>();
    assert(input_system);
    render_component_manager = engine.get_manager<mino::Manager<mino::RenderComponent>>();
    assert(render_component_manager);

    init_cells();
    generate_labyrinth();
    logger->info("MapSystem started successfully");
}

void MapSystem::update()
{
    for (auto& event : input_system->get_events())
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

void MapSystem::init_cells()
{
    logger->info("Initializing labyrinth");
    mino::PositionComponent map_offset{75, 50};
    for (auto y = 0; y < MAP_SIZE_Y; ++y)
    {
        // Each row should start at `q = -y / 2` so the map renders
        // as a sort of rectangular shape
        auto offset = -y / 2;
        for (auto x = 0; x < MAP_SIZE_X; ++x)
        {
            auto entity = engine.add_entity();
            auto& hex_component = hex_component_manager->add_component(entity.id);
            hex_component.q = offset + x;
            hex_component.r = y;
            cells.add(hex_component, false);

            auto components = render_system->create_renderable_entity(entity);
            components.position = hex_component.as_position() + map_offset;
            components.render.texture = render_system->load_texture("data/wall.png");
            components.render.source = {0, 0, 256, 256};
            components.render.dest = {0, 0, HEX_TEXTURE_SIZE, HEX_TEXTURE_SIZE};
        }
    }
}

void MapSystem::update_cells()
{
    hex_component_manager->iter([&](auto const& entity, auto const& component) {
        auto n_neighbours = 0;
        for (auto& neighbour : component.all_neighbours())
        {
            auto r = cells.get(neighbour);
            if (r != nullptr && *r)
            {
                ++n_neighbours;
            }
        }
        auto r = cells.get(component);
        if (r == nullptr)
        {
            return;
        }
        auto const is_alive = *r;
        auto render_component = render_component_manager->get_component(entity);
        assert(render_component);
        if (is_alive)
        {
            if (n_neighbours < 1 || 2 < n_neighbours)
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

void MapSystem::generate_labyrinth()
{
    logger->info("Generating new labyrinth");

    std::mt19937 rng;
    rng.seed(std::random_device()());
    std::uniform_int_distribution<std::mt19937::result_type> dist(1, 255);
    hex_component_manager->iter([&](auto const& entity, auto const& component) {
        auto r = cells.get(component);
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

