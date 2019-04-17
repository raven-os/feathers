#pragma once

#include <array>
#include <variant>

#include "wm/Container.hpp"

#include "View.hpp"

namespace wm
{
  struct ClientData
  {
    View *view;

    /// Doesn't actually update size of windowData, only the surface itself
    void resize(WindowNodeIndex index, WindowTree &windowTree, std::array<uint16_t, 2u> size);
    /// Doesn't actually update position of windowData, only the surface itself
    void move(WindowNodeIndex index, WindowTree &windowTree, std::array<uint16_t, 2u> position);
    std::array<uint16_t, 2u> getPosition() const noexcept;
    std::array<uint16_t, 2u> getSize() const noexcept;
  };

  struct WindowData
  {
    // Rect rect;
    std::variant<Container, ClientData> data;

    void resize(WindowNodeIndex index, WindowTree &windowTree, std::array<uint16_t, 2u> size);
    void move(WindowNodeIndex index, WindowTree &windowTree, std::array<uint16_t, 2u> position);

    std::array<uint16_t, 2u> getPosition() const noexcept;
    std::array<uint16_t, 2u> getSize() const noexcept;
    
  };
};
