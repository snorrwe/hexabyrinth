#pragma once
#include "minomaly/component/position_component.hpp"
#include <array>
#include <iostream>

constexpr auto HEX_SIZE = 10;
constexpr auto SCREEN_SIZE_X = 1000;
constexpr auto SCREEN_SIZE_Y = 600;
constexpr auto MAP_SIZE_X = SCREEN_SIZE_X / HEX_SIZE;
constexpr auto MAP_SIZE_Y = SCREEN_SIZE_Y / HEX_SIZE;
constexpr int HEX_TEXTURE_SIZE = HEX_SIZE * 2;

enum class Direction : size_t {
    NE = 0,
    E = 1,
    SE = 2,
    SW = 3,
    W = 4,
    NW = 5,
};

struct HexComponent {
    int q = 0;
    int r = 0;

    constexpr auto neighbour(Direction direction) const -> HexComponent
    {
        constexpr auto axial_directions = std::array{
            HexComponent{ 1, -1 },
            HexComponent{ 1, 0 },
            HexComponent{ 0, 1 },
            HexComponent{ -1, 1 },
            HexComponent{ -1, 0 },
            HexComponent{ 0, -1 },
        };
        auto ax = axial_directions[static_cast<int>(direction)];
        return *this + ax;
    }

    constexpr auto operator+(HexComponent const& other) const -> HexComponent
    {
        return HexComponent{ q + other.q, r + other.r };
    }

    constexpr auto operator-(HexComponent const& other) const -> HexComponent
    {
        return HexComponent{ q - other.q, r - other.r };
    }

    constexpr auto as_position() const -> mino::PositionComponent
    {
        constexpr auto sqrt_3 = 1.732050807568877;
        auto x = HEX_SIZE * (sqrt_3 * q + sqrt_3 / 2 * r);
        auto y = HEX_SIZE * (3.0 / 2 * r);
        return mino::PositionComponent{ int(x), int(y) };
    }
};

