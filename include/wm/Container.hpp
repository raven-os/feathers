#pragma once

#include <vector>
#include <array>

#include "wm/WindowNodeIndex.hpp"

struct wl_resource;

namespace wm
{
  class WindowTree;
  class WindowData;
  class ClientData;

  enum class PlacementStyle
    {
     tilling,
     floaty
    };

  struct Rect
  {
    std::array<int16_t, 2u> position;
    std::array<uint16_t, 2u> size;
  };


  struct Container
  {
  private:
    uint16_t getChildWidth(WindowNodeIndex index, WindowTree &windowTree, WindowNodeIndex childIndex);
    void updateChildWidths(WindowNodeIndex index, WindowTree &windowTree);

    /// Doesn't actually update size of windowData, only the contents of the container
    void resize_impl(WindowNodeIndex index, WindowTree &windowTree, std::array<uint16_t, 2u> size);
    /// Doesn't actually update position of windowData, only the contents of the container
    void move_impl(WindowNodeIndex index, WindowTree &windowTree, std::array<int16_t, 2u> position);
  public:
    Rect rect;

    static constexpr bool const horizontalTiling{false};
    static constexpr bool const verticalTiling{!horizontalTiling};
    bool direction{horizontalTiling};

    void resize(WindowNodeIndex index, WindowTree &windowTree, std::array<uint16_t, 2u> size);
    void move(WindowNodeIndex index, WindowTree &windowTree, std::array<int16_t, 2u> position);

    WindowNodeIndex addChild(WindowNodeIndex index, WindowTree &windowTree, ClientData &&newChildWindowData);
    void removeChild(WindowNodeIndex index, WindowTree &windowTree, WindowNodeIndex childIndex);

    std::array<int16_t, 2u> getPosition() const noexcept;
    std::array<uint16_t, 2u> getSize() const noexcept;
  };
}
