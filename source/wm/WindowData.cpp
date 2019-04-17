#include "wm/WindowData.hpp"

namespace wm
{
  void WindowData::resize(WindowNodeIndex index, WindowTree &windowTree, std::array<uint16_t, 2u> size)
  {
    std::visit([&](auto &data)
	       {
		 data.resize(index, windowTree, size);
	       }, data);
  }

  void WindowData::move(WindowNodeIndex index, WindowTree &windowTree, std::array<uint16_t, 2u> position)
  {
    std::visit([&](auto &data)
	       {
		 data.move(index, windowTree, position);
	       }, data);
  }

  std::array<uint16_t, 2u> WindowData::getPosition() const noexcept
  {
    return std::visit([](auto &data) noexcept
		      {
			return data.getPosition();
		      }, data);
  }

  void ClientData::resize(WindowNodeIndex, WindowTree &, std::array<uint16_t, 2u> size)
  {
    wlr_xdg_toplevel_v6_set_size(view->xdg_surface, size[0], size[1]);
  }

  void ClientData::move(WindowNodeIndex, WindowTree &, std::array<uint16_t, 2u> position)
  {
    // todo: move xdg surface
    view->x = position[0];
    view->y = position[1];
  }

  std::array<uint16_t, 2u> ClientData::getPosition() const noexcept
  {
    return {view->x, view->y};
  }
}
