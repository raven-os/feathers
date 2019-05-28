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

    void resize(WindowNodeIndex index, WindowTree &windowTree, std::array<uint16_t, 2u> size);
    void move(WindowNodeIndex index, WindowTree &windowTree, std::array<int16_t, 2u> position);
    std::array<int16_t, 2u> getPosition() const noexcept;
    std::array<uint16_t, 2u> getSize() const noexcept;
  };

  struct WindowData
  {
    // Rect rect;
    std::variant<ClientData, Container> data;

    void resize(WindowNodeIndex index, WindowTree &windowTree, std::array<uint16_t, 2u> size);
    void move(WindowNodeIndex index, WindowTree &windowTree, std::array<int16_t, 2u> position);

    std::array<int16_t, 2u> getPosition() const noexcept;
    std::array<uint16_t, 2u> getSize() const noexcept;

    // Only call this if you're 100% sure it contains a container
    Container &getContainer() noexcept;
    Container const &getContainer() const noexcept;
  };
};
