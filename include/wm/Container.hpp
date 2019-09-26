#pragma once

#include <vector>
#include <array>

#include "wm/WindowNodeIndex.hpp"
#include "wm/ClientData.hpp"

#include "util/FixedPoint.hpp"

struct wl_resource;

namespace wm
{
  class WindowTree;
  class WindowData;

  struct Rect
  {
    std::array<FixedPoint<-4, int32_t>, 2u> position;
    std::array<FixedPoint<-4, uint32_t>, 2u> size;
  };

  struct Container
  {
  private:
    FixedPoint<-4, uint32_t> getChildWidth(WindowNodeIndex index, WindowTree &windowTree, WindowNodeIndex childIndex);

    /// Doesn't actually update size of windowData, only the contents of the container
    void resize_impl(WindowNodeIndex index, WindowTree &windowTree, std::array<FixedPoint<-4, uint32_t>, 2u> size);
    /// Doesn't actually update position of windowData, only the contents of the container
    void move_impl(WindowNodeIndex index, WindowTree &windowTree, std::array<FixedPoint<-4, int32_t>, 2u> position);
    /// Doesn't actually update position of windowData, only the contents of the container after start
    void move_after_impl(WindowTree &windowTree, WindowNodeIndex start, std::array<FixedPoint<-4, int32_t>, 2u> position);
  public:
    Container(Rect const &rect, bool direction = horizontalTiling) noexcept;
    Container(Container const &) = delete;
    Container(Container &&) = default;

    Rect rect;

    static constexpr bool const horizontalTiling{false};
    static constexpr bool const verticalTiling{!horizontalTiling};
    bool direction;

    void updateChildWidths(WindowNodeIndex index, WindowTree &windowTree);

    void resize(WindowNodeIndex index, WindowTree &windowTree, std::array<uint16_t, 2u> size);
    void resize(WindowNodeIndex index, WindowTree &windowTree, std::array<FixedPoint<-4, uint32_t>, 2u> size);
    void move(WindowNodeIndex index, WindowTree &windowTree, std::array<FixedPoint<-4, int32_t>, 2u> position);


    WindowNodeIndex addChild(WindowNodeIndex index, WindowTree &windowTree, ClientData &&newChildWindowData);
    WindowNodeIndex addChild(WindowNodeIndex index, WindowTree &windowTree, WindowNodeIndex prev, ClientData &&newChildWindowData);
    void removeChild(WindowNodeIndex index, WindowTree &windowTree, WindowNodeIndex childIndex);
    void changeDirection(WindowNodeIndex index, WindowTree &windowTree);

    std::array<FixedPoint<-4, int32_t>, 2u> getPosition() const noexcept;
    std::array<uint16_t, 2u> getSize() const noexcept;
    std::array<FixedPoint<-4, int32_t>, 2u> getMinSize(WindowNodeIndex index, WindowTree &windowTree) const noexcept;
  };
}
