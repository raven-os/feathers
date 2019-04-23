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

  void WindowData::move(WindowNodeIndex index, WindowTree &windowTree, std::array<int16_t, 2u> position)
  {
    std::visit([&](auto &data)
	       {
		 data.move(index, windowTree, position);
	       }, data);
  }

  std::array<int16_t, 2u> WindowData::getPosition() const noexcept
  {
    return std::visit([](auto &data) noexcept
		      {
			return data.getPosition();
		      }, data);
  }

  void ClientData::resize(WindowNodeIndex, WindowTree &, std::array<uint16_t, 2u> size)
  {
    struct wlr_box box[1];
    wlr_xdg_surface_v6_get_geometry(view->xdg_surface, box);

    wlr_xdg_toplevel_v6_set_size(view->xdg_surface, size[0], size[1]);
  }

  void ClientData::move(WindowNodeIndex, WindowTree &, std::array<int16_t, 2u> position)
  {
    struct wlr_box box[1];
    wlr_xdg_surface_v6_get_geometry(view->xdg_surface, box);

    view->x = position[0] - box->x;
    view->y = position[1] - box->y;
  }

  std::array<int16_t, 2u> ClientData::getPosition() const noexcept
  {
    struct wlr_box box[1];
    wlr_xdg_surface_v6_get_geometry(view->xdg_surface, box);

    return {view->x + box->x, view->y + box->y};
  }
}
