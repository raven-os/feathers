#pragma once

#include <array>
#include <variant>

#include "wm/ClientData.hpp"
#include "wm/Container.hpp"

#include "View.hpp"

namespace wm
{
  struct WindowData
  {
    // Rect rect;
    std::variant<View*, std::unique_ptr<Container>> data;

    WindowData() = default;
    WindowData(Container &&container);
    WindowData(View *);

    void resize(WindowNodeIndex index, WindowTree &windowTree, std::array<uint16_t, 2u> size);
    void move(WindowNodeIndex index, WindowTree &windowTree, std::array<FixedPoint<-4, int32_t>, 2u> position);

    std::array<FixedPoint<-4, int32_t>, 2u> getPosition() const noexcept;
    std::array<uint16_t, 2u> getSize() const noexcept;

    // Only call this if you're 100% sure it contains a container
    Container &getContainer() noexcept;
    Container const &getContainer() const noexcept;
  };
};
