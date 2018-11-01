#pragma once
#include "minomaly/common/aabb.hpp"
#include "minomaly/common/matrix.hpp"
#include "minomaly/common/point.hpp"
#include <array>
#include <memory>
#include <tuple>

template <typename T>
class Quadtree {
    using TStored = std::tuple<mino::Point, T>;

    // NE, SE, SW, NW
    std::array<std::unique_ptr<Quadtree<T>>, 4> children{};
    std::array<TStored, 4> items{};
    uint8_t length;
    mino::AABB constraints;

public:
    explicit Quadtree(mino::AABB&& constraints)
        : length(0)
        , constraints(std::move(constraints))
    {
    }

    auto add(mino::Point const& p, T const& value) -> bool
    {
        if (!constraints.contains(p)) {
            return false;
        }
        if (length + 1 < items.size()) {
            items[length++] = TStored{ p, value };
            return true;
        }
        if (children[0] == nullptr) {
            split();
        }
        for (auto& child : children) {
            if (child->add(p, value)) {
                return true;
            }
        }
        // This should not happen. This is a programming error in the Quadtree
        // implementation
        assert(0);
        return false;
    }

    auto get(mino::Point const& p) -> T*
    {
        if (!constraints.contains(p)) {
            return nullptr;
        }
        auto result = std::find_if(items.begin(), items.end(), [&](auto const& i) {
            return std::get<0>(i) == p;
        });
        if (result != items.end()) {
            return &std::get<1>(*result);
        }
        if (children[0] == nullptr) {
            return nullptr;
        }
        for (auto& child : children) {
            auto result = child->get(p);
            if (result != nullptr) {
                return result;
            }
        }
        return nullptr;
    }

private:
    auto split() -> void
    {
        auto const x = constraints.center.x();
        auto const y = constraints.center.y();
        auto const w = int(constraints.radius.x() * 0.5) + 1;
        auto const h = int(constraints.radius.y() * 0.5) + 1;

        auto const x0 = static_cast<int>(x + w);
        auto const x1 = static_cast<int>(x - w);

        auto const y0 = static_cast<int>(y + h);
        auto const y1 = static_cast<int>(y - h);

        children[0] = std::make_unique<Quadtree<T>>(AABB{ x0, y0, w, h });
        children[1] = std::make_unique<Quadtree<T>>(AABB{ x0, y1, w, h });
        children[2] = std::make_unique<Quadtree<T>>(AABB{ x1, y1, w, h });
        children[3] = std::make_unique<Quadtree<T>>(AABB{ x1, y0, w, h });
    }
};

